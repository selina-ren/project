/**********************************************************************
Author: Yu Ji 2015
Project: Light Field Rendering on OpenCL


********************************************************************/

//#include <cmath>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <malloc.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "opencl_parts.hpp"

#include "BasicReader.hpp"
#include "BasicMath.hpp"
#include "objloader.hpp"

std::vector<glm::vec3> vertices_scene;
std::vector<glm::vec3> vertices_disp;
std::vector<glm::vec2> uvs;

unsigned int FrameBuffer;
unsigned int DepthBuffer;
// Vertex Array Objects Identifiers
unsigned int vao_scene_all[2];
unsigned int uvbuffer;

unsigned int vertexbuffer_scene_all[2];
// Program and Shader Identifiers
unsigned int p, v, f;
GLuint p_s, v_s, f_s;

float zFar;
float Zfar_scale;

float projMatrix_geo[16];
float projMatrix_geo_left[16];
float projMatrix_geo_right[16];

float projMatrix_disp[16];
float viewMatrix_disp[16];

float viewMatrix_geo[16];

// depth map parameters
float depthmap_fov;
float depthmap_fov_left;
float depthmap_fov_right;
float depthmap_fl_left;
float depthmap_fl_right;

int depth_w, depth_h;
float   *rendered_depth;
float cam_pos[3];
float slab_center[3];
float cam_lookat[3];
float cam_right[3];
float cam_up[3];
float lf_slab_scale;
int depthFlag;


//standalone parameters
float yaw;
float pitch;
float tilt_total;
float yaw_total;
float depth_ch;
float fov_ch;
float nview_fov_ch;
float focal_plane_ch;
float R_kernel_ch;
float verfactor;
float horfactor;
int mouse_pre_x;
int mouse_pre_y;
int mouse_x;
int mouse_y;
bool leftButtonDown;
#define MOTION_THERS_ANG 2.0
#define MOTION_SPEED_ANG 4.0
#define MOTION_SPEED_SPA 10.0

// Vertex Attribute Locations
unsigned int vertexLoc, colorLoc;

// Uniform variable Locations
unsigned int projMatrixLoc, viewMatrixLoc, zFarLoc, depthFlagLoc, TextureID;

using namespace std;

#ifdef _WIN32
static HWND               gHwnd;
HDC                       gHdc;
HGLRC                     gGlCtx;
BOOL quit = FALSE;
MSG msg;
#else
GLXContext gGlCtxSep;
#define GLX_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB           0x2092
typedef GLXContext(*GLXCREATECONTEXTATTRIBSARBPROC)(Display*, GLXFBConfig,
	GLXContext, Bool, const int*);
Window          winSep;
Display         *displayNameSep;
XEvent          xevSep;
#endif


static bool g_bPrintf = true;
//LightField_Render_GL *LightField_Render_GL::lightfield_Render = NULL;

#ifdef _WIN32
LRESULT CALLBACK WndProc_SA(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{



	case WM_CREATE:
		return 0;

	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;

	case WM_DESTROY:
		return 0;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			PostQuitMessage(0);
			return 0;
		case 0x57: //'W'
			verfactor = 0.5;
			break;
		case 0x53://'S'
			verfactor = -0.5;
			break;
		case 0x41: //'A'
			horfactor = -0.5;
			break;
		case 0x44://'D'
			horfactor = 0.5;
			break;
		case VK_UP: //'up'
			focal_plane_ch = -0.01;
			break;
		case VK_DOWN: //'down'
			focal_plane_ch = 0.01;
			break;
		case VK_LEFT: //'left'
			R_kernel_ch = -1;
			break;
		case VK_RIGHT: //'right'
			R_kernel_ch = 1;
			break;

		case VK_OEM_COMMA: //'<'
			nview_fov_ch = -0.01;
			break;
		case VK_OEM_PERIOD: //'>'
			nview_fov_ch = 0.01;
			break;
			/*
			case VK_NUMPAD8: //'up'
			depth_ch = 0.001;
			break;
			case VK_NUMPAD2: //'down'
			depth_ch = -0.001;
			break;
			*/

		}
		return 0;

	case WM_MOUSEMOVE:

		// Retrieve mouse screen position
		mouse_x = (short)LOWORD(lParam);
		mouse_y = (short)HIWORD(lParam);

		leftButtonDown = wParam & MK_LBUTTON;

		if (leftButtonDown)
		{
			yaw = (float)(mouse_pre_x - mouse_x);
			if (abs(yaw) < MOTION_THERS_ANG)
				yaw = 0;


			pitch = (float)(mouse_pre_y - mouse_y);

			if (abs(pitch) < MOTION_THERS_ANG)
				pitch = 0;

			mouse_pre_x = mouse_x;
			mouse_pre_y = mouse_y;
		}
		else if (!leftButtonDown)
		{
			mouse_pre_x = mouse_x;
			mouse_pre_y = mouse_y;
		}
		return 0;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);

	}
}
#endif


#ifdef _WIN32
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	/*
	switch (message)
	{


		
	case WM_CREATE:
		return 0;

	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;

	case WM_DESTROY:
		return 0;
		
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			PostQuitMessage(0);
			return 0;
		}
		return 0;
		
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);

	}*/

	return 0;
}
#endif


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void LightField_Render_GL::dprintf(const char *fmt, ...)
{
	va_list args;
	char buffer[2048];

	va_start(args, fmt);
	vsprintf_s(buffer, fmt, args);
	va_end(args);

	if (g_bPrintf)
		printf("%s", buffer);

	OutputDebugStringA(buffer);
}

int LightField_Render_GL::initialize()
{

	// Call base class Initialize to get default configuration
	if (sampleArgs->initialize() != SDK_SUCCESS)
	{
		return SDK_FAILURE;
	}

	Option* iteration_option = new Option;
	CHECK_ALLOCATION(iteration_option, "Memory Allocation error. (iteration_option)");

	iteration_option->_sVersion = "i";
	iteration_option->_lVersion = "iterations";
	iteration_option->_description = "Number of iterations to execute kernel";
	iteration_option->_type = CA_ARG_INT;
	iteration_option->_value = &iterations;

	sampleArgs->AddOption(iteration_option);

	delete iteration_option;

	count_disp = 1;

	return SDK_SUCCESS;
}

int LightField_Render_GL::loadViewerConfig(std::string configName)
{
	std::ifstream in;
	std::string str;




	in.open(configName);


	string kk;

	while (!in.eof())
	{
		while (getline(in, str))
		{
			std::string::size_type begin = str.find_first_not_of(" \f\t\v");
			//Skips blank lines
			if (begin == std::string::npos)
				continue;
			std::string firstWord;
			try {
				firstWord = str.substr(0, str.find(" "));
			}
			catch (std::exception& e) {
				firstWord = str.erase(str.find_first_of(" "), str.find_first_not_of(" "));
			}
			std::transform(firstWord.begin(), firstWord.end(), firstWord.begin(), ::toupper);


			// using VR device??
			if (firstWord == "USING_HMD")
				using_HMD = stoi(str.substr(str.find(" ") + 1, str.length()));

			// window_resolution
			if (firstWord == "WINDOW_RESOLUTION_W")
				win_res_width = stoi(str.substr(str.find(" ") + 1, str.length()));
			if (firstWord == "WINDOW_RESOLUTION_H")
				win_res_height = stoi(str.substr(str.find(" ") + 1, str.length()));


		}


	}

	return SDK_SUCCESS;

}

int LightField_Render_GL::loadSceneConfig(std::string configName, std::string *mapLFImageName, std::string *mapDepImageName, std::string *objName)
{
	std::ifstream in;
	std::string str;



	in.open(configName);


	string kk;

	while (!in.eof())
	{
		while (getline(in, str))
		{
			std::string::size_type begin = str.find_first_not_of(" \f\t\v");
			//Skips blank lines
			if (begin == std::string::npos)
				continue;
			std::string firstWord;
			try {
				firstWord = str.substr(0, str.find(" "));
			}
			catch (std::exception& e) {
				firstWord = str.erase(str.find_first_of(" "), str.find_first_not_of(" "));
			}
			std::transform(firstWord.begin(), firstWord.end(), firstWord.begin(), ::toupper);

			// general file for viewer
			if (firstWord == "LF_SOURCE_FILE")
				*mapLFImageName = str.substr(str.find(" ") + 1, str.length());
			if (firstWord == "DEPTH_SOURCE_FILE")
				*mapDepImageName = str.substr(str.find(" ") + 1, str.length());
			if (firstWord == "OBJ_SOURCE_FILE")
				*objName = str.substr(str.find(" ") + 1, str.length());
			if (firstWord == "OBJ_SOURCE_FILE")
				*objName = str.substr(str.find(" ") + 1, str.length());
			if (firstWord == "SLAB_CENTER_X")
				slab_center[0] = stof(str.substr(str.find(" ") + 1, str.length()));
			if (firstWord == "SLAB_CENTER_Y")
				slab_center[1] = stof(str.substr(str.find(" ") + 1, str.length()));
			if (firstWord == "SLAB_CENTER_Z")
				slab_center[2] = stof(str.substr(str.find(" ") + 1, str.length()));
			if (firstWord == "Z_FAR")
				zFar = stof(str.substr(str.find(" ") + 1, str.length()));
			if (firstWord == "LIGHT_FIELD_SLAB_SCALE")
				lf_slab_scale = stof(str.substr(str.find(" ") + 1, str.length()));


			if (firstWord == "LFCAM_FOV")
				lfcam_fov = stof(str.substr(str.find(" ") + 1, str.length()));
			
			if (firstWord == "NVIEW_FOCAL_LENGTH")
				nview_focal_length = stof(str.substr(str.find(" ") + 1, str.length()));
			if (firstWord == "NVIEW_FL_MAX")
				nview_fl_max = stof(str.substr(str.find(" ") + 1, str.length()));
			if (firstWord == "NVIEW_FL_MIN")
				nview_fl_min = stof(str.substr(str.find(" ") + 1, str.length()));

			if (firstWord == "HORIZONTAL_CAMERA_NUMBERS")
				lfcam_num_height = stoi(str.substr(str.find(" ") + 1, str.length()));
			if (firstWord == "VERTICAL_CAMERA_NUMBERS")
				lfcam_num_width = stoi(str.substr(str.find(" ") + 1, str.length()));

			if (firstWord == "DEPTH_SCAL")
				t_depth = stof(str.substr(str.find(" ") + 1, str.length()));
			if (firstWord == "DEPTH_MAX")
				depth_max = stof(str.substr(str.find(" ") + 1, str.length()));
			if (firstWord == "DEPTH_MIN")
				depth_min = stof(str.substr(str.find(" ") + 1, str.length()));

			if (firstWord == "LFCAM_RESOLUTION")
				lfcam_res = stoi(str.substr(str.find(" ") + 1, str.length()));
			if (firstWord == "NOVEL_VIEW_RESOLUTION")
				nview_res = stoi(str.substr(str.find(" ") + 1, str.length()));
		}


	}

	depthmap_fov = 2.0*(atan(0.5 / nview_focal_length))*180.0 / PI;

	

	
	

	return SDK_SUCCESS;

}

void LightField_Render_GL::vrProjection_Shift()
{
	depthmap_fov_left = 2.0*(atan(0.5 / nview_focal_length)) * 180 / PI;
	depthmap_fov_right = 2.0*(atan(0.5 / nview_focal_length)) * 180 / PI;

	depthmap_fl_left = nview_focal_length*2.0;
	depthmap_fl_right = nview_focal_length*2.0;

	float ratio = (float)nview_res_w / (float)nview_res_h;

	buildProjectionMatrix(depthmap_fov_left, ratio, 1.0, zFar, nview_shear_left_hor*2.0, nview_shear_left_ver*2.0, projMatrix_geo_left);
	buildProjectionMatrix(depthmap_fov_right, ratio, 1.0, zFar, nview_shear_right_hor*2.0, nview_shear_right_ver*2.0, projMatrix_geo_right);
	nview_ratio = 1.0 / ratio;
	nview_shear_left_ver_c = (nview_shear_left_ver - 0.5)*nview_ratio;
	nview_shear_left_hor_c = nview_shear_left_hor - 0.5;
	nview_shear_right_hor_c = nview_shear_right_hor - 0.5;
	nview_shear_right_ver_c = (nview_shear_right_ver - 0.5)*nview_ratio;
}

//void LightField_Render_GL::HandleInput(bool *bQuit, bool* xxx)
//{
//	SDL_Event sdlEvent;
//
//
//	while (SDL_PollEvent(&sdlEvent) != 0)
//	{
//		if (sdlEvent.type == SDL_QUIT)
//		{
//			*bQuit = true;
//		}
//		else if (sdlEvent.type == SDL_KEYDOWN)
//		{
//			if (sdlEvent.key.keysym.sym == SDLK_ESCAPE
//				|| sdlEvent.key.keysym.sym == SDLK_q)
//			{
//				*bQuit = true;
//			}
//		}
//	}
//}

int  LightField_Render_GL::rotate_cam(float ang, int axis)
{
	float ver_cos_d = cos((float)ang);
	float ver_sin_d = sin((float)ang);

	float nview_look_at_temp_s0 = nview_look_at.s0;
	float nview_look_at_temp_s1 = nview_look_at.s1;
	float nview_look_at_temp_s2 = nview_look_at.s2;

	float nview_up_temp_s0 = nview_up.s0;
	float nview_up_temp_s1 = nview_up.s1;
	float nview_up_temp_s2 = nview_up.s2;

	float nview_right_temp_s0 = nview_right.s0;
	float nview_right_temp_s1 = nview_right.s1;
	float nview_right_temp_s2 = nview_right.s2;

	if (axis == 0)
	{
		nview_look_at.s1 = nview_look_at_temp_s1 * ver_cos_d - nview_look_at_temp_s2 * ver_sin_d;
		nview_look_at.s2 = nview_look_at_temp_s1 * ver_sin_d + nview_look_at_temp_s2 * ver_cos_d;

		nview_up.s1 = nview_up_temp_s1 * ver_cos_d - nview_up_temp_s2 * ver_sin_d;
		nview_up.s2 = nview_up_temp_s1 * ver_sin_d + nview_up_temp_s2 * ver_cos_d;

		nview_right.s1 = nview_right_temp_s1 * ver_cos_d - nview_right_temp_s2 * ver_sin_d;
		nview_right.s2 = nview_right_temp_s1 * ver_sin_d + nview_right_temp_s2 * ver_cos_d;
	}
	else if (axis == 1)
	{
		nview_look_at.s0 = nview_look_at_temp_s0 * ver_cos_d - nview_look_at_temp_s2 * ver_sin_d;
		nview_look_at.s2 = nview_look_at_temp_s0 * ver_sin_d + nview_look_at_temp_s2 * ver_cos_d;

		nview_up.s0 = nview_up_temp_s0 * ver_cos_d - nview_up_temp_s2 * ver_sin_d;
		nview_up.s2 = nview_up_temp_s0 * ver_sin_d + nview_up_temp_s2 * ver_cos_d;

		nview_right.s0 = nview_right_temp_s0 * ver_cos_d - nview_right_temp_s2 * ver_sin_d;
		nview_right.s2 = nview_right_temp_s0 * ver_sin_d + nview_right_temp_s2 * ver_cos_d;
	}

	return SDK_SUCCESS;
}


void LightField_Render_GL::keybroad_action()
{
	if (depth_ch != 0)
	{
		t_depth += depth_ch;
		if (t_depth > depth_max)
			t_depth = depth_max;
		else if (t_depth < depth_min)
			t_depth = depth_min;
		depth_ch = 0;

		char str[256];
		sprintf_s(str, "the depth scale is %f\n", t_depth);


		OutputDebugString(str);

	}

	if (nview_fov_ch != 0)
	{
		nview_focal_length += nview_fov_ch;
		if (nview_focal_length > nview_fl_max)
			nview_focal_length = nview_fl_max;
		else if (nview_focal_length < nview_fl_min)
			nview_focal_length = nview_fl_min;
		nview_fov_ch = 0;
		depthmap_fov = 2.0*(atan(0.5 / nview_focal_length))*180.0 / PI;
		char str[256];
		sprintf_s(str, "the nview_fov is %f\n", nview_focal_length);
		OutputDebugString(str);

	}

	if (focal_plane_ch != 0)
	{
		d_focal += focal_plane_ch;
		if (d_focal > 0.9)
			d_focal = 0.9;
		else if (d_focal < 0.2)
			d_focal = 0.2;
		focal_plane_ch = 0;

		char str[256];
		sprintf_s(str, "the focal depth is %f\n", d_focal);


		OutputDebugString(str);

	}

	if (R_kernel_ch != 0)
	{
		psf_R += R_kernel_ch;
		if (psf_R >aper_max)
			psf_R = aper_max;
		else if (psf_R < aper_min)
			psf_R = aper_min;
		R_kernel_ch = 0;

		char str[256];
		sprintf_s(str, "the aperture size is %f\n", psf_R);


		OutputDebugString(str);

	}

}







void LightField_Render_GL::render_fn_display()
{
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//glEnable(GL_DEPTH_TEST);
	//glViewport(0, 0, nview_res, nview_res);





	//glDisable(GL_DEPTH_TEST);
	buildProjectionMatrix_SA(90, 1.0, 1.0f, zFar, projMatrix_disp);
	depthFlag = 0;

	float cam_position[3];
	float cam_direction[3];
	float cam_up[3];
	float cam_right[3];

	cam_position[0] = 0;
	cam_position[1] = 0;
	cam_position[2] = 1;


	cam_direction[0] = 0;
	cam_direction[1] = 0;
	cam_direction[2] = -1;

	cam_up[0] = 0;
	cam_up[1] = 1;
	cam_up[2] = 0;

	setCamera_SA(cam_position, cam_direction, cam_up, viewMatrix_disp);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex_depth);

	glUseProgram(p_s);
	setUniforms(projMatrix_disp, viewMatrix_disp, 100, 0);
	glUniform1i(TextureID, 0);

	glBegin(GL_QUADS);
	glVertex3f(-1.0f, -1.0f, 0.0f);
	//glTexCoord2f(1.0, 0.0);
	glVertexAttrib2f(colorLoc, 1.0, 0);
	glVertex3f(1.0f, -1.0f, 0.0f);
	//glTexCoord2f(1.0, 1.0);
	glVertexAttrib2f(colorLoc, 1.0, 1.0);
	glVertex3f(1.0f, 1.0f, 0.0f);
	//glTexCoord2f(0.0, 1.0);
	glVertexAttrib2f(colorLoc, 0, 1.0);
	glVertex3f(-1.0f, 1.0f, 0.0f);
	//glTexCoord2f(0.0, 0.0);
	glVertexAttrib2f(colorLoc, 0, 0);
	glEnd();

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glDisable(GL_TEXTURE_2D);
	SwapBuffers(gHdc);

	glFlush();
	glFinish();


}







int LightField_Render_GL::run(vr::IVRSystem *m_pHMD, vr::IVRCompositor *m_pCompositor)
{
	bool bQuit = false;

	while (!bQuit)
	{

		//HandleInput(&bQuit, &xxx);
	//while (!quit)
	//{
	//	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	//	{
	//		// handle or dispatch messages
	//		if (msg.message == WM_QUIT)
	//		{
	//			quit = TRUE;
	//		}
	//		else
	//		{
	//			TranslateMessage(&msg);
	//			DispatchMessage(&msg);
	//		}

	//	}
	//	else
	//	{

		
		//bQuit = pMainApplication->HandleInput();
		// left eye
		int left_flag = 1;
		float lookforward[3];
		float lookup[3];
		float lookright[3];
		float shiftPos[3];

		Matrix4 viewM = GetCurrentViewMatrix(matEyeToHeadLeft, m_mat4HMDPose);
		convert_Matrix2GeoDir(	viewM, 
								lookright, 
								lookup, 
								lookforward, 
								shiftPos);


		render_geo(	left_flag,
					lookright,
					lookup,
					lookforward,
					shiftPos);

		convert_Matrix2LFSDir(viewM);



		if (runCLKernels(left_flag) != SDK_SUCCESS)
		{
			return SDK_FAILURE;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, tex_fndisp);

		if (m_pCompositor)
		{
			m_pCompositor->Submit(vr::Eye_Left, vr::API_OpenGL, (void*)tex_fndisp, NULL);
		}


		//right eye
		left_flag = 0;
		viewM = GetCurrentViewMatrix(matEyeToHeadRight, m_mat4HMDPose);

		convert_Matrix2GeoDir(	viewM, 
								lookright, 
								lookup, 
								lookforward, 
								shiftPos);

		render_geo(	left_flag,
					lookright,
					lookup,
					lookforward,
					shiftPos);

		convert_Matrix2LFSDir(viewM);
		

		if (runCLKernels(left_flag) != SDK_SUCCESS)
		{
			return SDK_FAILURE;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, tex_fndisp);

		if (m_pCompositor)
		{
			m_pCompositor->Submit(vr::Eye_Right, vr::API_OpenGL, (void*)tex_fndisp, NULL);
		}


		glFinish();

		// Clear
		{
			// We want to make sure the glFinish waits for the entire present to complete, not just the submission
			// of the command. So, we do a clear here right here so the glFinish will wait fully for the swap.
			glClearColor(1.0, 1.0, 1.0, 1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}

		

		m_mat4HMDPose= UpdateHMDMatrixPose(m_pHMD, m_pCompositor);

	}



	return SDK_SUCCESS;
}

int LightField_Render_GL::run_SA()
{
	int status;

	// create and initialize timers
	int timer = sampleTimer->createTimer();
	sampleTimer->resetTimer(timer);
	sampleTimer->startTimer(timer);

	std::cout << "Executing kernel for " << iterations
		<< " iterations" << std::endl;
	std::cout << "-------------------------------------------" << std::endl;

	sampleTimer->stopTimer(timer);
	// Compute kernel time
	kernelTime = (double)(sampleTimer->readTimer(timer)) / iterations;

	/////////////////////////////////////////
	// OpenGL Here

	if (!sampleArgs->quiet && !sampleArgs->verify)
	{
		std::cout << "\nPress key w to move forward \n";
		std::cout << "Press key s to move backward \n";
		std::cout << "\nPress key a to move left \n";
		std::cout << "Press key d to move right \n";
		std::cout << "\nPress mouse to Yaw/Pitch \n";
		std::cout << "\nPress \"<\" to focus to foreground \n";
		std::cout << "Press \">\" to focus to background \n";
		std::cout << "Press ESC key to quit \n";
#ifdef _WIN32
		// program main loop
		while (!quit)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				// handle or dispatch messages
				if (msg.message == WM_QUIT)
				{
					quit = TRUE;
				}
				else
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}

			}
			else
			{
				t1 = clock() * CLOCKS_PER_SEC;
				frameCount++;

				keybroad_action();

				calculate_cam_pos_dir();

				render_geo_SA();
				glFinish();
				// Execute the kernel
				if (runCLKernels_SA() != SDK_SUCCESS)
				{
					return SDK_FAILURE;
				}


				// Bind  texture
				render_fn_display();

			}
		}
#endif

	}

	/////////////////////////////////////////

	//// write the output image to bitmap file
	//status = writeOutputImage(LIGHT_FIELD_RENDER_OUTPUT);
	//CHECK_ERROR(status, SDK_SUCCESS, "write Output Image Failed");

	return SDK_SUCCESS;
}










int LightField_Render_GL::convert_Matrix2GeoDir(Matrix4 viewM, float* lookright, float* lookup, float* lookforward, float* shiftPos)
{

	lookright[0] = viewM[0];
	lookright[1] = viewM[4];
	lookright[2] = viewM[8];

	lookup[0] = viewM[1];
	lookup[1] = viewM[5];
	lookup[2] = viewM[9];

	lookforward[0] = -viewM[2];
	lookforward[1] = -viewM[6];
	lookforward[2] = -viewM[10];

	shiftPos[0] = -viewM[12];
	shiftPos[1] = -viewM[13];
	shiftPos[2] = -viewM[14];

	return SDK_SUCCESS;

}

int LightField_Render_GL::convert_Matrix2LFSDir(Matrix4 viewM)
{


	nview_right.s0 = -viewM[0];
	nview_right.s1 = viewM[4];
	nview_right.s2 = -viewM[8];

	nview_up.s0 = -viewM[1];
	nview_up.s1 = viewM[5];
	nview_up.s2 = -viewM[9];

	nview_look_at.s0 = viewM[2];
	nview_look_at.s1 = -viewM[6];
	nview_look_at.s2 = viewM[10];

	nview_pos.s0 = viewM[12];
	nview_pos.s1 = -viewM[13];
	nview_pos.s2 = viewM[14];

	return SDK_SUCCESS;

}

int LightField_Render_GL::readSceneGeo(std::string objName_scene)
{
	std::vector<glm::vec3> out_vertices;
	bool res;
	res = loadOBJ_wo_tex_norm(objName_scene, vertices_scene);
	//res = loadOBJ(objName_disp, vertices_scene, uvs);

	return SDK_SUCCESS;
}

int LightField_Render_GL::readImage(std::string mapLFImageName, std::string mapDepImageName)
{
	const char *Dstr = mapDepImageName.c_str();
	printf("Loading Depth Image file %s...\n", Dstr);

	// load input bitmap image
	cv::Mat depth_image;
	depth_image = cv::imread(mapDepImageName, CV_LOAD_IMAGE_GRAYSCALE);
	cv::flip(depth_image, depth_image, 0);

	printf("Done\n");

	const char *Sstr = mapLFImageName.c_str();
	printf("Loading Source Image file %s...\n", Sstr);

	char str[256];
	sprintf_s(str, "input image channel is %d\n", depth_image.channels());
	OutputDebugString(str);
	mapLFBitmap.load(mapLFImageName.c_str());
	//mapDepBitmap.load(mapDepImageName.c_str());
	printf("Done\n");

	// get width and height of input image

	height = mapLFBitmap.getHeight();
	width = mapLFBitmap.getWidth();

	image_desc.image_width = width;
	image_desc.image_height = height;

	// allocate memory for map image data
	mapLFImageData = (cl_uchar4*)malloc(width * height * sizeof(cl_uchar4));
	CHECK_ALLOCATION(mapLFImageData, "Failed to allocate memory! (mapImageData)");

	mapDepImageData = (cl_uchar*)malloc(width * height * sizeof(cl_uchar));
	CHECK_ALLOCATION(mapDepImageData, "Failed to allocate memory! (mapDepImageData)");



	// allocate memory for map image data
	outputImageData = (cl_uchar4*)malloc(nview_res_w * nview_res_h * sizeof(cl_uchar4));
	CHECK_ALLOCATION(outputImageData, "Failed to allocate memory! (outputImageData)");

	// initialize the Image data to NULL
	memset(outputImageData, 255, nview_res_w * nview_res_h * pixelSize);

	// get the pointer to pixel data
	pixelData = mapLFBitmap.getPixels();
	CHECK_ALLOCATION(pixelData, "Failed to read mapBitmap pixel Data!");

	// get the point to depth data
	depthData = depth_image.data;
	CHECK_ALLOCATION(depthData, "Failed to read mapBitmap pixel Data!");

	// Copy pixel data into mapImageData
	memcpy(mapLFImageData, pixelData, width * height * pixelSize);
	memcpy(mapDepImageData, depthData, width * height);

	//delete(&mapLFBitmap);


	return SDK_SUCCESS;
}


bool LightField_Render_GL::loadAllScenefiles(std::string scene_configName)
{
	int status;

	//std::string viewer_configName = "viewer_config.ini";
	//std::string scene_configName = "scene_config.ini";
	std::string lf_filePath_src;
	std::string dep_filePath_src;
	std::string obj_src;

	//status = loadViewerConfig(viewer_configName);
	status = loadSceneConfig(scene_configName, &lf_filePath_src, &dep_filePath_src, &obj_src);

	std::string filePath_src = getPath() + lf_filePath_src;
	std::string depPath_src = getPath() + dep_filePath_src;
	std::string objPath_src = getPath() + obj_src;

	status = readSceneGeo(objPath_src);
	status = readImage(filePath_src, depPath_src);

	return true;
}

bool LightField_Render_GL::loadpsfConfig(std::string refocus_paraName, std::string * psfName)
{
	std::ifstream in;
	std::string str;



	in.open(refocus_paraName);


	string kk;

	while (!in.eof())
	{
		while (getline(in, str))
		{
			std::string::size_type begin = str.find_first_not_of(" \f\t\v");
			//Skips blank lines
			if (begin == std::string::npos)
				continue;
			std::string firstWord;
			try {
				firstWord = str.substr(0, str.find(" "));
			}
			catch (std::exception& e) {
				firstWord = str.erase(str.find_first_of(" "), str.find_first_not_of(" "));
			}
			std::transform(firstWord.begin(), firstWord.end(), firstWord.begin(), ::toupper);
			if (firstWord == "PSF_FILE")
				*psfName = str.substr(str.find(" ") + 1, str.length());
			if (firstWord == "FOCAL_DEPTH")
				d_focal = stof(str.substr(str.find(" ") + 1, str.length()));
			if (firstWord == "APERTURE_INIT")
				psf_R = stof(str.substr(str.find(" ") + 1, str.length()));
			if (firstWord == "APERTURE_MIN")
				aper_min = stof(str.substr(str.find(" ") + 1, str.length()));
			if (firstWord == "APERTURE_MAX")
				aper_max = stof(str.substr(str.find(" ") + 1, str.length()));
		}


	}

	return true;
}


bool LightField_Render_GL::loadRefocusPara(std::string refocus_paraName)
{
	int status;

	std::string psf_src;

	status = loadpsfConfig(refocus_paraName, &psf_src);
	std::string psfPath_src = getPath() + psf_src;

	readPSFImage(psfPath_src);

	return true;
}

bool LightField_Render_GL::readPSFImage(std::string psfImageName)
{
	mapPSFBitmap.load(psfImageName.c_str());

	if (!mapPSFBitmap.isLoaded())
	{
		error("Failed to load input image!");
		return SDK_FAILURE;
	}

	psf_h = mapPSFBitmap.getHeight();
	psf_w = mapPSFBitmap.getWidth();

	mapPSFImageData = (cl_uchar4*)malloc(psf_w * psf_h * sizeof(cl_uchar4));
	CHECK_ALLOCATION(mapLFImageData, "Failed to allocate memory! (mapImageData)");

	psfData = mapPSFBitmap.getPixels();
	CHECK_ALLOCATION(psfData, "Failed to read mapBitmap pixel Data!");

	memcpy(mapPSFImageData, psfData, psf_w * psf_w  *pixelSize);

	mapPSFBitmap.~SDKBitMap();

	return SDK_SUCCESS;

}


/**
*******************************************************************************
* @fn setupCL
* @brief  OpenCL related initialisations. Set up Context, Device list,
*    Command Queue, Memory buffers. Build CL kernel program executable
*
* @return int    : SDK_SUCCESS on success and SDK_FAILURE on failure
******************************************************************************/
int LightField_Render_GL::setupCL()
{
	cl_int status = CL_SUCCESS;
	cl_device_type dType;

	if (sampleArgs->deviceType.compare("cpu") == 0)
	{
		dType = CL_DEVICE_TYPE_CPU;
	}
	else //sampleArgs->deviceType = "gpu"
	{
		dType = CL_DEVICE_TYPE_GPU;
		if (sampleArgs->isThereGPU() == false)
		{
			std::cout << "GPU not found. Falling back to CPU device" << std::endl;
			dType = CL_DEVICE_TYPE_CPU;
		}
	}

	/*
	* Have a look at the available platforms and pick either
	* the AMD one if available or a reasonable default.
	*/
	cl_platform_id platform = NULL;
	int retValue = getPlatform(platform, sampleArgs->platformId,
		sampleArgs->isPlatformEnabled());
	CHECK_ERROR(retValue, SDK_SUCCESS, "getPlatform() failed");

	// Display available devices.
	retValue = displayDevices(platform, dType);
	CHECK_ERROR(retValue, SDK_SUCCESS, "displayDevices() failed");


	int success = enableGLAndGetGLContext(gHwnd, gHdc, gGlCtx, platform, context,
		interopDeviceId);
	if (SDK_SUCCESS != success)
	{
		if (success == SDK_EXPECTED_FAILURE)
		{
			return SDK_EXPECTED_FAILURE;
		}
		return SDK_FAILURE;
	}

	// getting device on which to run the sample
	// First, get the size of device list data
	size_t deviceListSize = 0;

	status = clGetContextInfo(
		context,
		CL_CONTEXT_DEVICES,
		0,
		NULL,
		&deviceListSize);
	CHECK_OPENCL_ERROR(status, "clGetContextInfo failed.");

	int deviceCount = (int)(deviceListSize / sizeof(cl_device_id));

	devices = (cl_device_id *)malloc(deviceListSize);
	CHECK_ALLOCATION((devices), "Failed to allocate memory (devices).");

	// Now, get the device list data
	status = clGetContextInfo(context,
		CL_CONTEXT_DEVICES,
		deviceListSize,
		(devices),
		NULL);
	CHECK_OPENCL_ERROR(status, "clGetGetContextInfo failed.");

	if (dType == CL_DEVICE_TYPE_CPU)
	{
		interopDeviceId = devices[sampleArgs->deviceId];
	}



	// Create command queue
	cl_command_queue_properties prop = 0;
	commandQueue = clCreateCommandQueue(
		context,
		interopDeviceId,
		prop,
		&status);
	CHECK_OPENCL_ERROR(status, "clCreateCommandQueue failed.");


	// Set input data

	/*
	* Create and initialize memory objects
	*/

	// Set Persistent memory only for AMD platform
	cl_mem_flags inMemFlags = CL_MEM_READ_ONLY;
	if (sampleArgs->isAmdPlatform())
	{
		inMemFlags |= CL_MEM_USE_PERSISTENT_MEM_AMD;
	}

	// Create memory object for input Image
	mapLFImage = clCreateBuffer(
		context,
		inMemFlags,
		width * height * pixelSize,
		NULL,
		&status);
	CHECK_OPENCL_ERROR(status, "clCreateBuffer failed. (inputImageBuffer)");


	cl_event writeEvt;
	status = clEnqueueWriteBuffer(
		commandQueue,
		mapLFImage,
		CL_FALSE,
		0,
		width * height * pixelSize,
		mapLFImageData,
		0,
		NULL,
		&writeEvt);
	CHECK_OPENCL_ERROR(status, "clEnqueueWriteBuffer failed. (inputImageBuffer)");

	status = clFlush(commandQueue);
	CHECK_OPENCL_ERROR(status, "clFlush failed.");

	status = waitForEventAndRelease(&writeEvt);
	CHECK_ERROR(status, SDK_SUCCESS, "WaitForEventAndRelease(writeEvt) Failed");

	///////////////////////////////

	/*
	* Create texture object
	*/

	glGenTextures(1, &tex_depth);
	glGenTextures(1, &tex_fndisp);
	

	//glBindTexture(GL_TEXTURE_2D, 0);


	glBindTexture(GL_TEXTURE_2D, tex_fndisp);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, nview_res_w, nview_res_h, 0, GL_RGBA,
		GL_UNSIGNED_BYTE, 0);
	// Set parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, tex_depth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, nview_res_w, nview_res_h, 0, GL_RGBA,
		GL_UNSIGNED_BYTE, 0);
	// Set parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_depth, 0);

	/*
	* Create clImage from GLTexture
	*/

	depth_map = clCreateFromGLTexture(context,
		CL_MEM_READ_ONLY,
		GL_TEXTURE_2D,
		0,
		tex_depth,
		&status);
	CHECK_OPENCL_ERROR(status, "clCreateImage failed. (depth_map)");

	outputImage = clCreateFromGLTexture(context,
		CL_MEM_READ_WRITE,
		GL_TEXTURE_2D,
		0,
		tex_fndisp,
		&status);
	CHECK_OPENCL_ERROR(status, "clCreateFromGLTexture failed. (outputImageBuffer)");

	mapDepImage = clCreateBuffer(
		context,
		inMemFlags,
		width*height,
		NULL,
		&status);
	CHECK_OPENCL_ERROR(status, "clCreateBuffer failed. (mapDepImage)");

	status = clEnqueueWriteBuffer(
		commandQueue,
		mapDepImage,
		CL_FALSE,
		0,
		width * height,
		mapDepImageData,
		0,
		NULL,
		&writeEvt);
	CHECK_OPENCL_ERROR(status, "clEnqueueWriteBuffer failed. (inputImageBuffer)");

	status = clFlush(commandQueue);
	CHECK_OPENCL_ERROR(status, "clFlush failed.");

	status = waitForEventAndRelease(&writeEvt);
	CHECK_ERROR(status, SDK_SUCCESS, "WaitForEventAndRelease(writeEvt) Failed");


	// create a CL program using the kernel source
	buildProgramData buildData;
	buildData.kernelName = std::string("LightField_Render_Kernels_VR_v1.cl");
	buildData.devices = devices;
	buildData.deviceId = sampleArgs->deviceId;
	buildData.flagsStr = std::string("");

	if (sampleArgs->isComplierFlagsSpecified())
	{
		buildData.flagsFileName = std::string(sampleArgs->flags.c_str());
	}

	if (sampleArgs->isLoadBinaryEnabled())
	{
		buildData.binaryName = std::string(sampleArgs->loadBinary.c_str());
	}




	retValue = buildOpenCLProgram(program, context, buildData);
	CHECK_ERROR(retValue, SDK_SUCCESS, "buildOpenCLProgram() failed");

	// get a kernel object handle for a kernel with the given name
	kernelLightField_Render = clCreateKernel(program, "LightField_ray_Render", &status);
	CHECK_OPENCL_ERROR(status, "clCreateKernel failed.(LightField_ray_Render)");



	status = kernelInfo.setKernelWorkGroupInfo(kernelLightField_Render, interopDeviceId);
	CHECK_ERROR(status, SDK_SUCCESS, "setKErnelWorkGroupInfo() failed");



	if ((blockSizeX * blockSizeY) > kernelInfo.kernelWorkGroupSize)
	{
		if (!sampleArgs->quiet)
		{
			std::cout << "Out of Resources!" << std::endl;
			std::cout << "Group Size specified : "
				<< blockSizeX * blockSizeY << std::endl;
			std::cout << "Max Group Size supported on the kernel : "
				<< kernelInfo.kernelWorkGroupSize << std::endl;
			std::cout << "Falling back to " << kernelInfo.kernelWorkGroupSize << std::endl;
		}

		// Three possible cases
		if (blockSizeX > kernelInfo.kernelWorkGroupSize)
		{
			blockSizeX = kernelInfo.kernelWorkGroupSize;
			blockSizeY = 1;
		}
	}



	return SDK_SUCCESS;
}

int LightField_Render_GL::setupCL_SA()
{
	cl_int status = CL_SUCCESS;
	cl_device_type dType;

	if (sampleArgs->deviceType.compare("cpu") == 0)
	{
		dType = CL_DEVICE_TYPE_CPU;
	}
	else //sampleArgs->deviceType = "gpu"
	{
		dType = CL_DEVICE_TYPE_GPU;
		if (sampleArgs->isThereGPU() == false)
		{
			std::cout << "GPU not found. Falling back to CPU device" << std::endl;
			dType = CL_DEVICE_TYPE_CPU;
		}
	}

	/*
	* Have a look at the available platforms and pick either
	* the AMD one if available or a reasonable default.
	*/
	cl_platform_id platform = NULL;
	int retValue = getPlatform(platform, sampleArgs->platformId,
		sampleArgs->isPlatformEnabled());
	CHECK_ERROR(retValue, SDK_SUCCESS, "getPlatform() failed");

	// Display available devices.
	retValue = displayDevices(platform, dType);
	CHECK_ERROR(retValue, SDK_SUCCESS, "displayDevices() failed");


	int success = enableGLAndGetGLContext_SA(gHwnd, gHdc, gGlCtx, platform, context,
		interopDeviceId);
	if (SDK_SUCCESS != success)
	{
		if (success == SDK_EXPECTED_FAILURE)
		{
			return SDK_EXPECTED_FAILURE;
		}
		return SDK_FAILURE;
	}



	// getting device on which to run the sample
	// First, get the size of device list data
	size_t deviceListSize = 0;

	status = clGetContextInfo(
		context,
		CL_CONTEXT_DEVICES,
		0,
		NULL,
		&deviceListSize);
	CHECK_OPENCL_ERROR(status, "clGetContextInfo failed.");

	int deviceCount = (int)(deviceListSize / sizeof(cl_device_id));

	devices = (cl_device_id *)malloc(deviceListSize);
	CHECK_ALLOCATION((devices), "Failed to allocate memory (devices).");

	// Now, get the device list data
	status = clGetContextInfo(context,
		CL_CONTEXT_DEVICES,
		deviceListSize,
		(devices),
		NULL);
	CHECK_OPENCL_ERROR(status, "clGetGetContextInfo failed.");

	if (dType == CL_DEVICE_TYPE_CPU)
	{
		interopDeviceId = devices[sampleArgs->deviceId];
	}



	// Create command queue
	cl_command_queue_properties prop = 0;
	commandQueue = clCreateCommandQueue(
		context,
		interopDeviceId,
		prop,
		&status);
	CHECK_OPENCL_ERROR(status, "clCreateCommandQueue failed.");


	///////////////////////////////

	/*
	* Create texture object
	*/



	/*
	* Create clImage from GLTexture
	*/


	depth_map = clCreateFromGLTexture(context,
		CL_MEM_READ_ONLY,
		GL_TEXTURE_2D,
		0,
		tex_depth,
		&status);
	CHECK_OPENCL_ERROR(status, "clCreateImage failed. (depth_map)");

	outputImage = clCreateFromGLTexture(context,
		CL_MEM_READ_WRITE,
		GL_TEXTURE_2D,
		0,
		tex_fndisp,
		&status);
	CHECK_OPENCL_ERROR(status, "clCreateFromGLTexture failed. (outputImageBuffer)");




	/*
	* Create and initialize memory objects
	*/

	// Set Persistent memory only for AMD platform
	cl_mem_flags inMemFlags = CL_MEM_READ_ONLY;
	if (sampleArgs->isAmdPlatform())
	{
		inMemFlags |= CL_MEM_USE_PERSISTENT_MEM_AMD;
	}

	// Create memory object for input Image
	mapLFImage = clCreateBuffer(
		context,
		inMemFlags,
		width * height * pixelSize,
		NULL,
		&status);
	CHECK_OPENCL_ERROR(status, "clCreateBuffer failed. (inputImageBuffer)");

	// Set input data
	cl_event writeEvt;
	status = clEnqueueWriteBuffer(
		commandQueue,
		mapLFImage,
		CL_FALSE,
		0,
		width * height * pixelSize,
		mapLFImageData,
		0,
		NULL,
		&writeEvt);
	CHECK_OPENCL_ERROR(status, "clEnqueueWriteBuffer failed. (inputImageBuffer)");

	status = clFlush(commandQueue);
	CHECK_OPENCL_ERROR(status, "clFlush failed.");

	status = waitForEventAndRelease(&writeEvt);
	CHECK_ERROR(status, SDK_SUCCESS, "WaitForEventAndRelease(writeEvt) Failed");

	mapDepImage = clCreateBuffer(
		context,
		inMemFlags,
		width*height,
		NULL,
		&status);
	CHECK_OPENCL_ERROR(status, "clCreateBuffer failed. (mapDepImage)");

	status = clEnqueueWriteBuffer(
		commandQueue,
		mapDepImage,
		CL_FALSE,
		0,
		width * height,
		mapDepImageData,
		0,
		NULL,
		&writeEvt);
	CHECK_OPENCL_ERROR(status, "clEnqueueWriteBuffer failed. (inputImageBuffer)");

	status = clFlush(commandQueue);
	CHECK_OPENCL_ERROR(status, "clFlush failed.");

	status = waitForEventAndRelease(&writeEvt);
	CHECK_ERROR(status, SDK_SUCCESS, "WaitForEventAndRelease(writeEvt) Failed");


	///////////////////////////////

	psfImage = clCreateBuffer(
		context,
		inMemFlags,
		psf_h * psf_w * pixelSize,
		0,
		&status);
	CHECK_OPENCL_ERROR(status, "clCreateBuffer failed. (tempImage)");

	// Set input data
	writeEvt;
	status = clEnqueueWriteBuffer(
		commandQueue,
		psfImage,
		CL_FALSE,
		0,
		psf_w * psf_h * pixelSize,
		mapPSFImageData,
		0,
		NULL,
		&writeEvt);
	CHECK_OPENCL_ERROR(status, "clEnqueueWriteBuffer failed. (inputImageBuffer)");

	status = clFlush(commandQueue);
	CHECK_OPENCL_ERROR(status, "clFlush failed.");

	status = waitForEventAndRelease(&writeEvt);
	CHECK_ERROR(status, SDK_SUCCESS, "WaitForEventAndRelease(writeEvt) Failed");

	// Create memory object for temp Image
	tempImage = clCreateBuffer(
		context,
		CL_MEM_READ_WRITE,
		nview_res * nview_res * pixelSize,
		0,
		&status);
	CHECK_OPENCL_ERROR(status, "clCreateBuffer failed. (tempImage)");


	// create a CL program using the kernel source
	buildProgramData buildData;
	buildData.kernelName = std::string("LightField_Render_Kernels_Standalone_v1.cl");
	buildData.devices = devices;
	buildData.deviceId = sampleArgs->deviceId;
	buildData.flagsStr = std::string("");

	if (sampleArgs->isComplierFlagsSpecified())
	{
		buildData.flagsFileName = std::string(sampleArgs->flags.c_str());
	}

	if (sampleArgs->isLoadBinaryEnabled())
	{
		buildData.binaryName = std::string(sampleArgs->loadBinary.c_str());
	}




	retValue = buildOpenCLProgram(program, context, buildData);
	CHECK_ERROR(retValue, SDK_SUCCESS, "buildOpenCLProgram() failed");

	// get a kernel object handle for a kernel with the given name
	kernelLightField_Render = clCreateKernel(program, "LightField_ray_Render", &status);
	CHECK_OPENCL_ERROR(status, "clCreateKernel failed.(LightField_ray_Render)");

	kernelRefocusing = clCreateKernel(program, "Refocusing_kernel", &status);
	CHECK_OPENCL_ERROR(status, "clCreateKernel failed.(Refocusing_kernel)");

	status = kernelInfo.setKernelWorkGroupInfo(kernelLightField_Render, interopDeviceId);
	CHECK_ERROR(status, SDK_SUCCESS, "setKErnelWorkGroupInfo() failed");

	status = kernelInfo.setKernelWorkGroupInfo(kernelRefocusing, interopDeviceId);
	CHECK_ERROR(status, SDK_SUCCESS, "setKErnelWorkGroupInfo() failed");


	if ((blockSizeX * blockSizeY) > kernelInfo.kernelWorkGroupSize)
	{
		if (!sampleArgs->quiet)
		{
			std::cout << "Out of Resources!" << std::endl;
			std::cout << "Group Size specified : "
				<< blockSizeX * blockSizeY << std::endl;
			std::cout << "Max Group Size supported on the kernel : "
				<< kernelInfo.kernelWorkGroupSize << std::endl;
			std::cout << "Falling back to " << kernelInfo.kernelWorkGroupSize << std::endl;
		}

		// Three possible cases
		if (blockSizeX > kernelInfo.kernelWorkGroupSize)
		{
			blockSizeX = kernelInfo.kernelWorkGroupSize;
			blockSizeY = 1;
		}
	}



	return SDK_SUCCESS;
}





void LightField_Render_GL::setupBuffers_scene(std::vector<glm::vec3> vertices_geo, std::vector<glm::vec3> vertices_disp)
{


	glGenVertexArrays(2, &vao_scene_all[0]);

	glBindVertexArray(vao_scene_all[0]);
	// Generate two slots for the vertex and color buffers

	glGenBuffers(2, &vertexbuffer_scene_all[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_scene_all[0]);

	//int xx = vertices.size() * sizeof(glm::vec3);

	glBufferData(GL_ARRAY_BUFFER, vertices_geo.size() * sizeof(glm::vec3), &vertices_geo[0], GL_STATIC_DRAW);

	glBindVertexArray(vao_scene_all[1]);
	// Generate two slots for the vertex and color buffers

	//glGenBuffers(1, &vertexbuffer_scene_all[1]);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_scene_all[1]);
	glBufferData(GL_ARRAY_BUFFER, vertices_disp.size() * sizeof(glm::vec3), &vertices_disp[0], GL_STATIC_DRAW);


	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);






}

void LightField_Render_GL::changeSize(int w, int h)
{

	float ratio;
	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if (h == 0)
		h = 1;

	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);

	ratio = (1.0f * w) / h;
	buildProjectionMatrix(depthmap_fov, ratio, 1.0f, zFar, 0, 0, projMatrix_geo);


}








void LightField_Render_GL::buildProjectionMatrix(float fov, float ratio, float nearP, float farP, float hor_shear, float ver_shear, float* projMatrix)
{

	float f = 1.0f / tan(fov * (M_PI / 360.0));

	setIdentityMatrix(projMatrix, 4);

	projMatrix[0] = f ;
	projMatrix[1 * 4 + 1] = f * ratio;
	projMatrix[2 * 4 + 2] = (farP + nearP) / (nearP - farP);
	projMatrix[3 * 4 + 2] = (2.0f * farP * nearP) / (nearP - farP);
	projMatrix[2 * 4 + 3] = -1.0f;
	projMatrix[3 * 4 + 3] = 0.0f;
	projMatrix[8] = hor_shear;
	projMatrix[9] = ver_shear;
}

void LightField_Render_GL::buildProjectionMatrix_SA(float fov, float ratio, float nearP, float farP, float* projMatrix)
{

	float f = 1.0f / tan(fov * (M_PI / 360.0));

	setIdentityMatrix(projMatrix, 4);

	projMatrix[0] = f / ratio;
	projMatrix[1 * 4 + 1] = f;
	projMatrix[2 * 4 + 2] = (farP + nearP) / (nearP - farP);
	projMatrix[3 * 4 + 2] = (2.0f * farP * nearP) / (nearP - farP);
	projMatrix[2 * 4 + 3] = -1.0f;
	projMatrix[3 * 4 + 3] = 0.0f;
}







GLuint LightField_Render_GL::setupShaders()
{

	char *vs = NULL, *fs = NULL, *fs2 = NULL;

	//unsigned int p_temp, v_temp, f_temp;

	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);

	char *vertexFileName;
	char *fragmentFileName;
	vertexFileName = "color.vert";
	fragmentFileName = "color.frag";

	vs = textFileRead(vertexFileName);
	fs = textFileRead(fragmentFileName);

	const char * vv = vs;
	const char * ff = fs;

	glShaderSource(v, 1, &vv, NULL);
	glShaderSource(f, 1, &ff, NULL);


	free(vs); free(fs);

	glCompileShader(v);
	glCompileShader(f);

	printShaderInfoLog(v);
	printShaderInfoLog(f);

	p = glCreateProgram();
	glAttachShader(p, v);
	glAttachShader(p, f);

	glBindFragDataLocation(p, 0, "outputF");
	glLinkProgram(p);
	printProgramInfoLog(p);

	vertexLoc = glGetAttribLocation(p, "position");
	colorLoc = glGetAttribLocation(p, "uv");

	float zFar;

	projMatrixLoc = glGetUniformLocation(p, "projMatrix");
	viewMatrixLoc = glGetUniformLocation(p, "viewMatrix");
	zFarLoc = glGetUniformLocation(p, "Zfar_scale");
	depthFlagLoc = glGetUniformLocation(p, "depth_flag");
	TextureID = glGetUniformLocation(p, "myTexture");

	p_s = p;
	v_s = v;
	f_s = f;

	return(p);
}






int LightField_Render_GL::openGL_Init()
{
	glewInit();
	if (!glewIsSupported("GL_VERSION_2_0 " "GL_ARB_pixel_buffer_object"))
	{
		std::cout << "Support for necessary OpenGL extensions missing."
			<< std::endl;
		return SDK_FAILURE;
	}

	p = setupShaders();

	setupBuffers_scene(vertices_scene, vertices_disp);
	//setupBuffers_display(vertices_disp);


	glClearColor(0.0, 0.0, .50, 1.0);
	glEnable(GL_TEXTURE_2D);

	//
	glGenFramebuffers(1, &FrameBuffer);
	glGenRenderbuffers(1, &DepthBuffer);



	glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, DepthBuffer);

	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, nview_res_w, nview_res_h);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, DepthBuffer);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glClearDepth(1.0f);


	//glBindFramebuffer(GL_FRAMEBUFFER, 0);

	Zfar_scale = 1.0 / zFar;

	changeSize(nview_res_w, nview_res_h);


	return SDK_SUCCESS;
}

int LightField_Render_GL::openGL_Init_SA()
{
	glewInit();
	if (!glewIsSupported("GL_VERSION_2_0 " "GL_ARB_pixel_buffer_object"))
	{
		std::cout << "Support for necessary OpenGL extensions missing."
			<< std::endl;
		return SDK_FAILURE;
	}



	p = setupShaders();

	setupBuffers_scene(vertices_scene, vertices_disp);
	//setupBuffers_display(vertices_disp);


	glClearColor(0.0, 0.0, .50, 1.0);
	glEnable(GL_TEXTURE_2D);

	//
	glGenFramebuffers(1, &FrameBuffer);
	glGenRenderbuffers(1, &DepthBuffer);



	glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, DepthBuffer);

	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, nview_res, nview_res);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, DepthBuffer);


	glGenTextures(1, &tex_depth);
	glGenTextures(1, &tex_fndisp);




	//glBindTexture(GL_TEXTURE_2D, 0);


	glBindTexture(GL_TEXTURE_2D, tex_fndisp);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, nview_res, nview_res, 0, GL_RGBA,
		GL_UNSIGNED_BYTE, 0);
	// Set parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, tex_depth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, nview_res, nview_res, 0, GL_RGBA,
		GL_UNSIGNED_BYTE, 0);
	// Set parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_depth, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	Zfar_scale = 1.0 / zFar;

	changeSize(nview_res, nview_res);


	return SDK_SUCCESS;
}








void LightField_Render_GL::render_geo(int left_flag, float* cam_right, float* cam_up, float* cam_direction, float* cam_position)
{

	convert_camera_pos_dir(cam_position, cam_direction, cam_up, cam_right);
	setCamera(cam_position, cam_direction, cam_up, cam_right, viewMatrix_geo);


	glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);


	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_depth, 0);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glClearDepth(1.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	

	glUseProgram(p_s);
	depthFlag = 1;
	if (left_flag)
	{
		setUniforms(projMatrix_geo_left, viewMatrix_geo, Zfar_scale, 1);
	}
	else
	{
		setUniforms(projMatrix_geo_right, viewMatrix_geo, Zfar_scale, 1);
	}


	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_scene_all[0]);
	glVertexAttribPointer(
		0,                  // attribute
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
		);

	glBindVertexArray(vao_scene_all[0]);
	glDrawArrays(GL_TRIANGLES, 0, vertices_scene.size());




	//glDisableVertexAttribArray(0);



	glFlush();
	glFinish();

	


}

void LightField_Render_GL::render_geo_SA()
{
	float cam_position[3];
	float cam_direction[3];
	float cam_up[3];
	float cam_right[3];
	convert_camera_pos_dir_SA(cam_position, cam_direction, cam_up, cam_right);

	glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_fndisp, 0);

	/*glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glClearDepth(1.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	*/
	buildProjectionMatrix_SA(depthmap_fov, 1.0, 1.0f, zFar, projMatrix_geo);
	setCamera_SA(cam_position, cam_direction, cam_up, viewMatrix_geo);

	glUseProgram(p_s);
	depthFlag = 1;
	setUniforms(projMatrix_geo, viewMatrix_geo, Zfar_scale, 1);


	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_scene_all[0]);
	glVertexAttribPointer(
		0,                  // attribute
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
		);

	glBindVertexArray(vao_scene_all[0]);
	glDrawArrays(GL_TRIANGLES, 0, vertices_scene.size());


	//glDisableVertexAttribArray(0);


	glFlush();
}






void LightField_Render_GL::setUniforms(float* projMatrix, float* viewMatrix, float Zfar_scale, int depthFlag)
{

	// must be called after glUseProgram
	glUniformMatrix4fv(projMatrixLoc, 1, false, projMatrix);
	glUniformMatrix4fv(viewMatrixLoc, 1, false, viewMatrix);
	glUniform1f(zFarLoc, Zfar_scale);
	glUniform1i(depthFlagLoc, depthFlag);
}







void LightField_Render_GL::setCamera(float* pos, float* look_dir, float* up_dir, float* right_dir, float* viewMatrix)
{

	float dir[3], right[3], up[3];

	/*
	up[0] = cam_up[0];
	up[1] = cam_up[1];
	up[2] = cam_up[2];
	*/

	up[0] = up_dir[0];
	up[1] = up_dir[1];
	up[2] = up_dir[2];
	//normalize(up);
	//up[0] = 0.0f;   up[1] = 1.0f;   up[2] = 0.0f;

	dir[0] = look_dir[0];
	dir[1] = look_dir[1];
	dir[2] = look_dir[2];
	normalize(dir);


	crossProduct(dir, up, right);
	normalize(right);

	crossProduct(right, dir, up);
	normalize(up);

	//crossProduct(right, dir, up);


	float aux[16];
	viewMatrix[0] = right[0];
	viewMatrix[4] = right[1];
	viewMatrix[8] = right[2];
	viewMatrix[12] = 0.0f;

	viewMatrix[1] = up[0];
	viewMatrix[5] = up[1];
	viewMatrix[9] = up[2];
	viewMatrix[13] = 0.0f;

	viewMatrix[2] = -dir[0];
	viewMatrix[6] = -dir[1];
	viewMatrix[10] = -dir[2];
	viewMatrix[14] = 0.0f;

	viewMatrix[3] = 0.0f;
	viewMatrix[7] = 0.0f;
	viewMatrix[11] = 0.0f;
	viewMatrix[15] = 1.0f;

	setTranslationMatrix(aux, -pos[0], -pos[1], -pos[2]);

	multMatrix(viewMatrix, aux);

}

void LightField_Render_GL::setCamera_SA(float* pos, float* look_dir, float* up_dir, float* viewMatrix)
{

	float dir[3], right[3], up[3];

	/*
	up[0] = cam_up[0];
	up[1] = cam_up[1];
	up[2] = cam_up[2];
	*/

	up[0] = up_dir[0];
	up[1] = up_dir[1];
	up[2] = up_dir[2];

	//up[0] = 0.0f;   up[1] = 1.0f;   up[2] = 0.0f;

	dir[0] = look_dir[0];
	dir[1] = look_dir[1];
	dir[2] = look_dir[2];
	normalize(dir);

	crossProduct(dir, up, right);
	normalize(right);

	crossProduct(right, dir, up);
	normalize(up);

	float aux[16];
	viewMatrix[0] = right[0];
	viewMatrix[4] = right[1];
	viewMatrix[8] = right[2];
	viewMatrix[12] = 0.0f;

	viewMatrix[1] = up[0];
	viewMatrix[5] = up[1];
	viewMatrix[9] = up[2];
	viewMatrix[13] = 0.0f;

	viewMatrix[2] = -dir[0];
	viewMatrix[6] = -dir[1];
	viewMatrix[10] = -dir[2];
	viewMatrix[14] = 0.0f;

	viewMatrix[3] = 0.0f;
	viewMatrix[7] = 0.0f;
	viewMatrix[11] = 0.0f;
	viewMatrix[15] = 1.0f;

	setTranslationMatrix(aux, -pos[0], -pos[1], -pos[2]);

	multMatrix(viewMatrix, aux);

}





void LightField_Render_GL::convert_camera_pos_dir(float* cam_position, float* cam_direction, float* cam_up, float* cam_right)
{
	///////////////// need to be done
	float temp[3];

	temp[0] = cam_position[2] * lf_slab_scale + slab_center[0];
	temp[1] = cam_position[1] * lf_slab_scale + slab_center[1];
	temp[2] = -cam_position[0] * lf_slab_scale + slab_center[2];

	cam_position[0] = temp[0];
	cam_position[1] = temp[1];
	cam_position[2] = temp[2];

	temp[0] = cam_direction[2];
	temp[1] = cam_direction[1];
	temp[2] = -cam_direction[0];

	cam_direction[0] = temp[0];
	cam_direction[1] = temp[1];
	cam_direction[2] = temp[2];

	temp[0] = cam_right[2];
	temp[1] = cam_right[1];
	temp[2] = -cam_right[0];

	cam_right[0] = temp[0];
	cam_right[1] = temp[1];
	cam_right[2] = temp[2];

	temp[0] = cam_up[2];
	temp[1] = cam_up[1];
	temp[2] = -cam_up[0];

	cam_up[0] = temp[0];
	cam_up[1] = temp[1];
	cam_up[2] = temp[2];
}

void LightField_Render_GL::convert_camera_pos_dir_SA(float* cam_position, float* cam_direction, float* cam_up, float* cam_right)
{
	///////////////// need to be done
	cam_position[0] = nview_pos.s2*lf_slab_scale + slab_center[0];
	cam_position[1] = nview_pos.s1*lf_slab_scale + slab_center[1];
	cam_position[2] = -nview_pos.s0*lf_slab_scale + slab_center[2];

	cam_direction[0] = nview_look_at.s2;
	cam_direction[1] = nview_look_at.s1;
	cam_direction[2] = -nview_look_at.s0;

	cam_right[0] = nview_right.s2;
	cam_right[1] = nview_right.s1;
	cam_right[2] = -nview_right.s0;

	cam_up[0] = nview_up.s2;
	cam_up[1] = nview_up.s1;
	cam_up[2] = -nview_up.s0;
}







void LightField_Render_GL::calculate_cam_pos_dir()
{
	//compute camera location & direction
	if (verfactor != 0)
	{
		float nview_pos_s0_temp = nview_pos.s0 + nview_look_at.s0 * (float)verfactor / MOTION_SPEED_SPA;
		float nview_pos_s2_temp = nview_pos.s2 + nview_look_at.s2 * (float)verfactor / MOTION_SPEED_SPA;
		if ((abs(nview_pos_s0_temp) <= 1) && (abs(nview_pos_s2_temp) <= 1))
		{
			nview_pos.s0 = nview_pos_s0_temp;
			nview_pos.s2 = nview_pos_s2_temp;
		}
		verfactor = 0;
	}
	if (horfactor != 0)
	{
		float nview_pos_s0_temp = nview_pos.s0 + nview_right.s0 * (float)horfactor / MOTION_SPEED_SPA;
		float nview_pos_s2_temp = nview_pos.s2 + nview_right.s2 * (float)horfactor / MOTION_SPEED_SPA;
		if ((abs(nview_pos_s0_temp) <= 1) && (abs(nview_pos_s2_temp) <= 1))
		{
			nview_pos.s0 = nview_pos_s0_temp;
			nview_pos.s2 = nview_pos_s2_temp;
		}
		horfactor = 0;
	}
	if (yaw != 0)
	{
		float rot_ang = yaw*2.0 / nview_res*atan(0.5 / nview_focal_length);

		rotate_cam(rot_ang, 1);

		yaw_total += rot_ang;
		yaw = 0;
	}

	if (pitch != 0)
	{
		float rot_ang = pitch*2.0 / nview_res*atan(0.5 / nview_focal_length);

		tilt_total += rot_ang;
		if (abs(tilt_total) > PI / 2.0)
		{
			tilt_total -= rot_ang;
		}
		else
		{
			rotate_cam(-yaw_total, 1);
			rotate_cam(-rot_ang, 0);
			rotate_cam(yaw_total, 1);
		}

		pitch = 0;
	}
}







int LightField_Render_GL::runCLKernels(int left_flag)
{
	cl_int status;

	cl_event acquireEvt_input;


	nview_fl = nview_focal_length;
	nview_shear_hor = -0.5;
	nview_shear_ver = -0.619289339;

	status = clEnqueueAcquireGLObjects(commandQueue,
		1,
		&depth_map,
		0,
		NULL,
		&acquireEvt_input);

	CHECK_OPENCL_ERROR(status, "clCreateFromGLTexture failed. (depth_map)");
	status = clFlush(commandQueue);
	CHECK_OPENCL_ERROR(status, "clFlush failed.");

	status = waitForEventAndRelease(&acquireEvt_input);
	CHECK_ERROR(status, SDK_SUCCESS, "WaitForEventAndRelease(acquireEvt) Failed");

	//Acquire GL buffer
	cl_event acquireEvt_output;
	status = clEnqueueAcquireGLObjects(commandQueue,
		1,
		&outputImage,
		0,
		NULL,
		&acquireEvt_output);
	CHECK_OPENCL_ERROR(status, "clEnqueueAcquireGLObjects failed.");

	status = clFlush(commandQueue);
	CHECK_OPENCL_ERROR(status, "clFlush failed.");

	status = waitForEventAndRelease(&acquireEvt_output);
	CHECK_ERROR(status, SDK_SUCCESS, "WaitForEventAndRelease(acquireEvt) Failed");



	float depth_scal;


	depth_scal = t_depth * zFar / lf_slab_scale;

	// input buffer image
	status = clSetKernelArg(
		kernelLightField_Render,
		0,
		sizeof(cl_mem),
		&mapLFImage);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (mapLFImage)");

	// input depth Light Field Slab image
	status = clSetKernelArg(
		kernelLightField_Render,
		1,
		sizeof(cl_mem),
		&mapDepImage);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (mapLFImage)");

	// depth_map image

	status = clSetKernelArg(
		kernelLightField_Render,
		2,
		sizeof(cl_mem),
		&depth_map);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (depth_map)");

	// outBuffer image
	status = clSetKernelArg(
		kernelLightField_Render,
		3,
		sizeof(cl_mem), // resolution might be change!!!!
		&outputImage);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (outputImage)");


	// light field sub-aperture image resolution
	status = clSetKernelArg(
		kernelLightField_Render,
		4,
		sizeof(cl_int),
		&lfcam_res);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (lfcam_res)");

	// light field sub-aperture image number in height
	status = clSetKernelArg(
		kernelLightField_Render,
		5,
		sizeof(cl_int),
		&lfcam_num_height);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (lfcam_num_height)");

	// light field sub-aperture image number in width
	status = clSetKernelArg(
		kernelLightField_Render,
		6,
		sizeof(cl_int),
		&lfcam_num_width);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (lfcam_num_width)");


	// light field camera focal_length
	status = clSetKernelArg(
		kernelLightField_Render,
		7,
		sizeof(cl_float),
		&lfcam_fov);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (lfcam_fov)");

	// render novel view resolution
	status = clSetKernelArg(
		kernelLightField_Render,
		8,
		sizeof(cl_int),
		&nview_res_w);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (nview_res_w)");

	// render novel view resolution
	status = clSetKernelArg(
		kernelLightField_Render,
		9,
		sizeof(cl_float),
		&nview_fl);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (nview_focal_length)");

	// novel view lookat direction
	status = clSetKernelArg(
		kernelLightField_Render,
		10,
		sizeof(cl_float4),
		&nview_look_at);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (nview_look_at)");

	// novel view camera up direction
	status = clSetKernelArg(
		kernelLightField_Render,
		11,
		sizeof(cl_float4),
		&nview_up);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (nview_up)");

	// novel view camera right direction
	status = clSetKernelArg(
		kernelLightField_Render,
		12,
		sizeof(cl_float4),
		&nview_right);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (nview_right)");

	// novel view camera postion
	status = clSetKernelArg(
		kernelLightField_Render,
		13,
		sizeof(cl_float4),
		&nview_pos);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (nview_pos)");

	cl_int color_coded_slabs = 0;
	// render color-coded slabs
	status = clSetKernelArg(
		kernelLightField_Render,
		14,
		sizeof(cl_bool),
		&color_coded_slabs);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (color_coded_slabs)");


	// render color-coded slabs
	status = clSetKernelArg(
		kernelLightField_Render,
		15,
		sizeof(cl_float),
		&depth_scal);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (depth_scal)");

	// render color-coded slabs
	cl_int show_depth = 0;
	status = clSetKernelArg(
		kernelLightField_Render,
		16,
		sizeof(cl_bool),
		&show_depth);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (show_depth)");

	// nview_shear_hor
	status = clSetKernelArg(
		kernelLightField_Render,
		17,
		sizeof(cl_float),
		&nview_shear_hor);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (nview_shear_hor)");

	// nview_shear_hor
	status = clSetKernelArg(
		kernelLightField_Render,
		18,
		sizeof(cl_float),
		&nview_shear_ver);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (nview_shear_ver)");

	// Enqueue a kernel run call.
	size_t globalThreads[] = { nview_res_w, nview_res_h }; // need to change the resolution Yu Ji
	size_t localThreads[] = { blockSizeX, blockSizeY }; // need to change the resolution Yu Ji

	cl_event ndrEvt2;

	status = clEnqueueNDRangeKernel(
		commandQueue,
		kernelLightField_Render,
		2,
		NULL,
		globalThreads,
		localThreads,
		0,
		NULL,
		&ndrEvt2);
	CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel failed.");

	status = clFinish(commandQueue);
	CHECK_OPENCL_ERROR(status, "clFinish failed.");

	status = waitForEventAndRelease(&ndrEvt2);
	CHECK_ERROR(status, SDK_SUCCESS, "WaitForEventAndRelease(ndrEvt2) Failed");



	// Now OpenGL gets control of outputImageBuffer

	cl_event releaseEvt_input;
	status = clEnqueueReleaseGLObjects(commandQueue,
		1,
		&depth_map,
		0,
		NULL,
		&releaseEvt_input);
	CHECK_OPENCL_ERROR(status, "clEnqueueReleaseGLObjects failed. (depth_map)");
	status = clFlush(commandQueue);
	CHECK_OPENCL_ERROR(status, "clFlush failed.");

	status = waitForEventAndRelease(&releaseEvt_input);
	CHECK_ERROR(status, SDK_SUCCESS, "WaitForEventAndRelease(acquireEvt) Failed");



	cl_event releaseGLEvt_output;

	status = clEnqueueReleaseGLObjects(commandQueue,
		1,
		&outputImage,
		0,
		NULL,
		&releaseGLEvt_output);

	CHECK_OPENCL_ERROR(status, "clEnqueueReleaseGLObjects failed.");

	status = clFlush(commandQueue);
	CHECK_OPENCL_ERROR(status, "clFlush failed.");

	status = waitForEventAndRelease(&releaseGLEvt_output);
	CHECK_ERROR(status, SDK_SUCCESS, "WaitForEventAndRelease(releaseGLEvt) Failed");


	return SDK_SUCCESS;
}

int LightField_Render_GL::runCLKernels_SA()
{
	cl_int status;

	cl_event acquireEvt_input;

	status = clEnqueueAcquireGLObjects(commandQueue,
		1,
		&depth_map,
		0,
		NULL,
		&acquireEvt_input);

	CHECK_OPENCL_ERROR(status, "clCreateFromGLTexture failed. (depth_map)");
	status = clFlush(commandQueue);
	CHECK_OPENCL_ERROR(status, "clFlush failed.");

	status = waitForEventAndRelease(&acquireEvt_input);
	CHECK_ERROR(status, SDK_SUCCESS, "WaitForEventAndRelease(acquireEvt) Failed");

	//Acquire GL buffer
	cl_event acquireEvt_output;
	status = clEnqueueAcquireGLObjects(commandQueue,
		1,
		&outputImage,
		0,
		NULL,
		&acquireEvt_output);
	CHECK_OPENCL_ERROR(status, "clEnqueueAcquireGLObjects failed.");

	status = clFlush(commandQueue);
	CHECK_OPENCL_ERROR(status, "clFlush failed.");

	status = waitForEventAndRelease(&acquireEvt_output);
	CHECK_ERROR(status, SDK_SUCCESS, "WaitForEventAndRelease(acquireEvt) Failed");



	float depth_scal;


	depth_scal = t_depth * zFar / lf_slab_scale;

	// input buffer image
	status = clSetKernelArg(
		kernelLightField_Render,
		0,
		sizeof(cl_mem),
		&mapLFImage);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (mapLFImage)");


	// input depth Light Field Slab image
	status = clSetKernelArg(
		kernelLightField_Render,
		1,
		sizeof(cl_mem),
		&mapDepImage);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (mapLFImage)");

	// depth_map image

	status = clSetKernelArg(
		kernelLightField_Render,
		2,
		sizeof(cl_mem),
		&depth_map);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (depth_map)");


	// outBuffer image
	status = clSetKernelArg(
		kernelLightField_Render,
		3,
		sizeof(cl_mem), // resolution might be change!!!!
		&tempImage);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (outputImage)");


	// light field sub-aperture image resolution
	status = clSetKernelArg(
		kernelLightField_Render,
		4,
		sizeof(cl_int),
		&lfcam_res);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (lfcam_res)");

	// light field sub-aperture image number in height
	status = clSetKernelArg(
		kernelLightField_Render,
		5,
		sizeof(cl_int),
		&lfcam_num_height);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (lfcam_num_height)");

	// light field sub-aperture image number in width
	status = clSetKernelArg(
		kernelLightField_Render,
		6,
		sizeof(cl_int),
		&lfcam_num_width);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (lfcam_num_width)");


	// light field camera focal_length
	status = clSetKernelArg(
		kernelLightField_Render,
		7,
		sizeof(cl_float),
		&lfcam_fov);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (lfcam_fov)");

	// render novel view resolution
	status = clSetKernelArg(
		kernelLightField_Render,
		8,
		sizeof(cl_int),
		&nview_res);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (nview_res)");

	// render novel view resolution
	status = clSetKernelArg(
		kernelLightField_Render,
		9,
		sizeof(cl_float),
		&nview_focal_length);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (nview_focal_length)");

	// novel view lookat direction
	status = clSetKernelArg(
		kernelLightField_Render,
		10,
		sizeof(cl_float4),
		&nview_look_at);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (nview_look_at)");

	// novel view camera up direction
	status = clSetKernelArg(
		kernelLightField_Render,
		11,
		sizeof(cl_float4),
		&nview_up);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (nview_up)");

	// novel view camera right direction
	status = clSetKernelArg(
		kernelLightField_Render,
		12,
		sizeof(cl_float4),
		&nview_right);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (nview_right)");

	// novel view camera postion
	status = clSetKernelArg(
		kernelLightField_Render,
		13,
		sizeof(cl_float4),
		&nview_pos);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (nview_pos)");





	// render color-coded slabs
	status = clSetKernelArg(
		kernelLightField_Render,
		14,
		sizeof(cl_float),
		&depth_scal);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (depth_scal)");



	// Enqueue a kernel run call.
	size_t globalThreads[] = { nview_res, nview_res }; // need to change the resolution Yu Ji
	size_t localThreads[] = { blockSizeX, blockSizeY }; // need to change the resolution Yu Ji

	cl_event ndrEvt2;

	status = clEnqueueNDRangeKernel(
		commandQueue,
		kernelLightField_Render,
		2,
		NULL,
		globalThreads,
		localThreads,
		0,
		NULL,
		&ndrEvt2);
	CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel failed.");

	status = clFinish(commandQueue);
	CHECK_OPENCL_ERROR(status, "clFinish failed.");

	status = waitForEventAndRelease(&ndrEvt2);
	CHECK_ERROR(status, SDK_SUCCESS, "WaitForEventAndRelease(ndrEvt2) Failed");



	/* outBuffer imager */
	status = clSetKernelArg(
		kernelRefocusing,
		0,
		sizeof(cl_mem),
		&psfImage);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (outputImageBuffer)");

	status = clSetKernelArg(
		kernelRefocusing,
		1,
		sizeof(cl_mem),
		&depth_map);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (depth_map)");

	status = clSetKernelArg(
		kernelRefocusing,
		2,
		sizeof(cl_mem),
		&tempImage);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (depth_map)");

	status = clSetKernelArg(
		kernelRefocusing,
		3,
		sizeof(cl_mem),
		&outputImage);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (depth_map)");

	status = clSetKernelArg(
		kernelRefocusing,
		4,
		sizeof(cl_float),
		&d_focal);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (d_focal)");

	status = clSetKernelArg(
		kernelRefocusing,
		5,
		sizeof(cl_float),
		&psf_R);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (psf_R)");

	status = clSetKernelArg(
		kernelRefocusing,
		6,
		sizeof(cl_int),
		&nview_res);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (nview_res)");

	status = clSetKernelArg(
		kernelRefocusing,
		7,
		sizeof(cl_int),
		&nview_res);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (nview_res)");

	ndrEvt2;
	status = clEnqueueNDRangeKernel(
		commandQueue,
		kernelRefocusing,
		2,
		NULL,
		globalThreads,
		localThreads,
		0,
		NULL,
		&ndrEvt2);
	CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel failed.");

	status = clFlush(commandQueue);
	CHECK_OPENCL_ERROR(status, "clFlush failed.");

	status = waitForEventAndRelease(&ndrEvt2);
	CHECK_ERROR(status, SDK_SUCCESS, "WaitForEventAndRelease(ndrEvt2) Failed");

	status = clFinish(commandQueue);
	CHECK_OPENCL_ERROR(status, "clFinish failed.");

	// Now OpenGL gets control of outputImageBuffer

	cl_event releaseEvt_input;
	status = clEnqueueReleaseGLObjects(commandQueue,
		1,
		&depth_map,
		0,
		NULL,
		&releaseEvt_input);
	CHECK_OPENCL_ERROR(status, "clEnqueueReleaseGLObjects failed. (depth_map)");
	status = clFlush(commandQueue);
	CHECK_OPENCL_ERROR(status, "clFlush failed.");

	status = waitForEventAndRelease(&releaseEvt_input);
	CHECK_ERROR(status, SDK_SUCCESS, "WaitForEventAndRelease(acquireEvt) Failed");



	cl_event releaseGLEvt_output;

	status = clEnqueueReleaseGLObjects(commandQueue,
		1,
		&outputImage,
		0,
		NULL,
		&releaseGLEvt_output);

	CHECK_OPENCL_ERROR(status, "clEnqueueReleaseGLObjects failed.");

	status = clFlush(commandQueue);
	CHECK_OPENCL_ERROR(status, "clFlush failed.");

	status = waitForEventAndRelease(&releaseGLEvt_output);
	CHECK_ERROR(status, SDK_SUCCESS, "WaitForEventAndRelease(releaseGLEvt) Failed");


	return SDK_SUCCESS;
}









int LightField_Render_GL::cleanup()
{


	// Releases OpenCL resources (Context, Memory etc.)
	cl_int status;

	status = clReleaseKernel(kernelLightField_Render);
	CHECK_OPENCL_ERROR(status, "clReleaseKernel failed.(kernelLightField_Render)");

	status = clReleaseProgram(program);
	CHECK_OPENCL_ERROR(status, "clReleaseProgram failed.(program)");

	status = clReleaseMemObject(mapLFImage);
	CHECK_OPENCL_ERROR(status, "clReleaseMemObject failed.(mapLFImage)");

	status = clReleaseMemObject(depth_map);
	CHECK_OPENCL_ERROR(status, "clReleaseMemObject failed.(depth_map)");

	status = clReleaseMemObject(outputImage);
	CHECK_OPENCL_ERROR(status, "clReleaseMemObject failed.(outputImage)");

	status = clReleaseCommandQueue(commandQueue);
	CHECK_OPENCL_ERROR(status, "clReleaseCommandQueue failed.(commandQueue)");

	status = clReleaseContext(context);
	CHECK_OPENCL_ERROR(status, "clReleaseContext failed.(context)");



	return SDK_SUCCESS;
}

int LightField_Render_GL::cleanup_SA()
{
#ifdef _WIN32
	wglDeleteContext(gGlCtx);
	DeleteDC(gHdc);
	gHdc = NULL;
	gGlCtx = NULL;
	DestroyWindow(gHwnd);
#endif

	// Releases OpenCL resources (Context, Memory etc.)
	cl_int status;

	status = clReleaseKernel(kernelLightField_Render);
	CHECK_OPENCL_ERROR(status, "clReleaseKernel failed.(kernelLightField_Render)");

	status = clReleaseProgram(program);
	CHECK_OPENCL_ERROR(status, "clReleaseProgram failed.(program)");

	status = clReleaseMemObject(mapLFImage);
	CHECK_OPENCL_ERROR(status, "clReleaseMemObject failed.(mapLFImage)");

	status = clReleaseMemObject(tempImage);
	CHECK_OPENCL_ERROR(status, "clReleaseMemObject failed.(tempImage)");

	status = clReleaseMemObject(depth_map);
	CHECK_OPENCL_ERROR(status, "clReleaseMemObject failed.(depth_map)");

	status = clReleaseMemObject(outputImage);
	CHECK_OPENCL_ERROR(status, "clReleaseMemObject failed.(outputImage)");

	status = clReleaseCommandQueue(commandQueue);
	CHECK_OPENCL_ERROR(status, "clReleaseCommandQueue failed.(commandQueue)");

	status = clReleaseContext(context);
	CHECK_OPENCL_ERROR(status, "clReleaseContext failed.(context)");

#ifndef _WIN32
	glXMakeCurrent(displayNameSep, None, NULL);
	glXDestroyContext(displayNameSep, gGlCtxSep);
	glDeleteTextures(1, &tex);
	XDestroyWindow(displayNameSep, winSep);
	XCloseDisplay(displayNameSep);
#endif

	// release program resources (input memory etc.)

	FREE(mapLFImageData);
	FREE(outputImageData);
	FREE(devices);

	return SDK_SUCCESS;
}






#ifdef _WIN32
int
LightField_Render_GL::enableGLAndGetGLContext(HWND hWnd,HDC &hDC,HGLRC &hRC,
cl_platform_id platform,cl_context &context,cl_device_id &interopDevice)
{
	cl_int status;
	BOOL ret = FALSE;
	DISPLAY_DEVICE dispDevice;
	DWORD deviceNum;
	int  pfmt;
	PIXELFORMATDESCRIPTOR  pfd;
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cRedBits = 8;
	pfd.cRedShift = 0;
	pfd.cGreenBits = 8;
	pfd.cGreenShift = 0;
	pfd.cBlueBits = 8;
	pfd.cBlueShift = 0;
	pfd.cAlphaBits = 8;
	pfd.cAlphaShift = 0;
	pfd.cAccumBits = 0;
	pfd.cAccumRedBits = 0;
	pfd.cAccumGreenBits = 0;
	pfd.cAccumBlueBits = 0;
	pfd.cAccumAlphaBits = 0;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 8;
	pfd.cAuxBuffers = 0;
	pfd.iLayerType = PFD_MAIN_PLANE;
	pfd.bReserved = 0;
	pfd.dwLayerMask = 0;
	pfd.dwVisibleMask = 0;
	pfd.dwDamageMask = 0;

	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

	dispDevice.cb = sizeof(DISPLAY_DEVICE);

	DWORD connectedDisplays = 0;
	DWORD displayDevices = 0;
	int xCoordinate = 100;
	int yCoordinate = 100;
	int xCoordinate1 = 100;
	for (deviceNum = 0; EnumDisplayDevices(NULL, deviceNum, &dispDevice, 0);
		deviceNum++)
	{
		if (dispDevice.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER)
		{
			continue;
		}

		if (!(dispDevice.StateFlags & DISPLAY_DEVICE_ACTIVE))
		{
			continue;
		}

		DEVMODE deviceMode;

		// initialize the DEVMODE structure
		ZeroMemory(&deviceMode, sizeof(deviceMode));
		deviceMode.dmSize = sizeof(deviceMode);
		deviceMode.dmDriverExtra = 0;


		EnumDisplaySettings(dispDevice.DeviceName, ENUM_CURRENT_SETTINGS, &deviceMode);

		//xCoordinate = deviceMode.dmPosition.x;
		//yCoordinate = deviceMode.dmPosition.y;

		WNDCLASS windowclass;

		windowclass.style = CS_OWNDC;
		windowclass.lpfnWndProc = WndProc;
		windowclass.cbClsExtra = 0;
		windowclass.cbWndExtra = 0;
		windowclass.hInstance = NULL;
		windowclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		windowclass.hCursor = LoadCursor(NULL, IDC_ARROW);
		windowclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		windowclass.lpszMenuName = NULL;
		windowclass.lpszClassName = reinterpret_cast<LPCSTR>("LightField_Render_GL");
		RegisterClass(&windowclass);

		gHwnd = CreateWindow(reinterpret_cast<LPCSTR>("LightField_Render_GL"),
			reinterpret_cast<LPCSTR>("LightField_Render_GL"),
			WS_CAPTION | WS_POPUPWINDOW,
			sampleArgs->isDeviceIdEnabled() ? xCoordinate1 : xCoordinate,
			yCoordinate,
			1280,
			720,
			NULL,
			NULL,
			windowclass.hInstance,
			NULL);
		hDC = GetDC(gHwnd);

		pfmt = ChoosePixelFormat(hDC,
			&pfd);
		if (pfmt == 0)
		{
			std::cout << "Failed choosing the requested PixelFormat.\n";
			return SDK_FAILURE;
		}

		ret = SetPixelFormat(hDC, pfmt, &pfd);

		if (ret == FALSE)
		{
			std::cout << "Failed to set the requested PixelFormat.\n";
			return SDK_FAILURE;
		}

		hRC = wglCreateContext(hDC);
		if (hRC == NULL)
		{
			std::cout << "Failed to create a GL context" << std::endl;
			return SDK_FAILURE;
		}

		ret = wglMakeCurrent(hDC, hRC);
		if (ret == FALSE)
		{
			std::cout << "Failed to bind GL rendering context";
			return SDK_FAILURE;
		}
		displayDevices++;

		cl_context_properties properties[] =
		{
			CL_CONTEXT_PLATFORM, (cl_context_properties)platform,
			CL_GL_CONTEXT_KHR, (cl_context_properties)hRC,
			CL_WGL_HDC_KHR, (cl_context_properties)hDC,
			0
		};

		if (!clGetGLContextInfoKHR)
		{
			clGetGLContextInfoKHR = (clGetGLContextInfoKHR_fn)
				clGetExtensionFunctionAddressForPlatform(platform, "clGetGLContextInfoKHR");
			if (!clGetGLContextInfoKHR)
			{
				std::cout << "Failed to query proc address for clGetGLContextInfoKHR";
				return SDK_FAILURE;
			}
		}

		size_t deviceSize = 0;
		status = clGetGLContextInfoKHR(properties,
			CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR,
			0,
			NULL,
			&deviceSize);
		if (status != CL_SUCCESS)
		{
			std::cout << "clGetGLContextInfoKHR failed!!" << std::endl;
			return SDK_FAILURE;
		}

		if (deviceSize == 0)
		{
			// no interopable CL device found, cleanup
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(hRC);
			DeleteDC(hDC);
			hDC = NULL;
			hRC = NULL;
			DestroyWindow(gHwnd);
			// try the next display
			continue;
		}
		else
		{
			if (sampleArgs->deviceId == 0)
			{
				ShowWindow(gHwnd, SW_SHOW);
				//Found a winner
				break;
			}
			else if (sampleArgs->deviceId != connectedDisplays)
			{
				connectedDisplays++;
				wglMakeCurrent(NULL, NULL);
				wglDeleteContext(hRC);
				DeleteDC(hDC);
				hDC = NULL;
				hRC = NULL;
				DestroyWindow(gHwnd);
				if (xCoordinate >= 0)
				{
					xCoordinate1 += deviceMode.dmPelsWidth;
					// try the next display
				}
				else
				{
					xCoordinate1 -= deviceMode.dmPelsWidth;
				}

				continue;
			}
			else
			{
				ShowWindow(gHwnd, SW_SHOW);
				//Found a winner
				break;
			}
		}
	}

	if (!hRC || !hDC)
	{
		OPENCL_EXPECTED_ERROR("OpenGL interoperability is not feasible.");
	}

	cl_context_properties properties[] =
	{
		CL_CONTEXT_PLATFORM, (cl_context_properties)platform,
		CL_GL_CONTEXT_KHR, (cl_context_properties)hRC,
		CL_WGL_HDC_KHR, (cl_context_properties)hDC,
		0
	};

	if (sampleArgs->deviceType.compare("gpu") == 0)
	{
		status = clGetGLContextInfoKHR(properties,
			CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR,
			sizeof(cl_device_id),
			&interopDevice,
			NULL);
		CHECK_OPENCL_ERROR(status, "clGetGLContextInfoKHR failed!!");

		// Create OpenCL context from device's id
		context = clCreateContext(properties,
			1,
			&interopDevice,
			0,
			0,
			&status);
		CHECK_OPENCL_ERROR(status, "clCreateContext failed!!");
	}
	else
	{
		context = clCreateContextFromType(
			properties,
			CL_DEVICE_TYPE_CPU,
			NULL,
			NULL,
			&status);
		CHECK_OPENCL_ERROR(status, "clCreateContextFromType failed!!");
	}


	////////////// OpenGL Init //////////////
	openGL_Init();
	////////////// OpenGL Init //////////////

}

int
LightField_Render_GL::enableGLAndGetGLContext_SA(HWND hWnd, HDC &hDC, HGLRC &hRC,
cl_platform_id platform, cl_context &context, cl_device_id &interopDevice)
{
	cl_int status;
	BOOL ret = FALSE;
	DISPLAY_DEVICE dispDevice;
	DWORD deviceNum;
	int  pfmt;
	PIXELFORMATDESCRIPTOR  pfd;
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cRedBits = 8;
	pfd.cRedShift = 0;
	pfd.cGreenBits = 8;
	pfd.cGreenShift = 0;
	pfd.cBlueBits = 8;
	pfd.cBlueShift = 0;
	pfd.cAlphaBits = 8;
	pfd.cAlphaShift = 0;
	pfd.cAccumBits = 0;
	pfd.cAccumRedBits = 0;
	pfd.cAccumGreenBits = 0;
	pfd.cAccumBlueBits = 0;
	pfd.cAccumAlphaBits = 0;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 8;
	pfd.cAuxBuffers = 0;
	pfd.iLayerType = PFD_MAIN_PLANE;
	pfd.bReserved = 0;
	pfd.dwLayerMask = 0;
	pfd.dwVisibleMask = 0;
	pfd.dwDamageMask = 0;

	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

	dispDevice.cb = sizeof(DISPLAY_DEVICE);

	DWORD connectedDisplays = 0;
	DWORD displayDevices = 0;
	int xCoordinate = 0;
	int yCoordinate = 0;
	int xCoordinate1 = 0;
	for (deviceNum = 0; EnumDisplayDevices(NULL, deviceNum, &dispDevice, 0);
		deviceNum++)
	{
		if (dispDevice.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER)
		{
			continue;
		}

		if (!(dispDevice.StateFlags & DISPLAY_DEVICE_ACTIVE))
		{
			continue;
		}

		DEVMODE deviceMode;

		// initialize the DEVMODE structure
		ZeroMemory(&deviceMode, sizeof(deviceMode));
		deviceMode.dmSize = sizeof(deviceMode);
		deviceMode.dmDriverExtra = 0;


		EnumDisplaySettings(dispDevice.DeviceName, ENUM_CURRENT_SETTINGS, &deviceMode);

		xCoordinate = deviceMode.dmPosition.x;
		yCoordinate = deviceMode.dmPosition.y;

		WNDCLASS windowclass;

		windowclass.style = CS_OWNDC;
		windowclass.lpfnWndProc = WndProc_SA;
		windowclass.cbClsExtra = 0;
		windowclass.cbWndExtra = 0;
		windowclass.hInstance = NULL;
		windowclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		windowclass.hCursor = LoadCursor(NULL, IDC_ARROW);
		windowclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		windowclass.lpszMenuName = NULL;
		windowclass.lpszClassName = reinterpret_cast<LPCSTR>("LightField_Render_GL");
		RegisterClass(&windowclass);

		gHwnd = CreateWindow(reinterpret_cast<LPCSTR>("LightField_Render_GL"),
			reinterpret_cast<LPCSTR>("LightField_Render_GL"),
			WS_CAPTION | WS_POPUPWINDOW,
			sampleArgs->isDeviceIdEnabled() ? xCoordinate1 : xCoordinate,
			yCoordinate,
			nview_res,
			nview_res,
			NULL,
			NULL,
			windowclass.hInstance,
			NULL);
		hDC = GetDC(gHwnd);

		pfmt = ChoosePixelFormat(hDC,
			&pfd);
		if (pfmt == 0)
		{
			std::cout << "Failed choosing the requested PixelFormat.\n";
			return SDK_FAILURE;
		}

		ret = SetPixelFormat(hDC, pfmt, &pfd);

		if (ret == FALSE)
		{
			std::cout << "Failed to set the requested PixelFormat.\n";
			return SDK_FAILURE;
		}

		hRC = wglCreateContext(hDC);
		if (hRC == NULL)
		{
			std::cout << "Failed to create a GL context" << std::endl;
			return SDK_FAILURE;
		}

		ret = wglMakeCurrent(hDC, hRC);
		if (ret == FALSE)
		{
			std::cout << "Failed to bind GL rendering context";
			return SDK_FAILURE;
		}
		displayDevices++;

		cl_context_properties properties[] =
		{
			CL_CONTEXT_PLATFORM, (cl_context_properties)platform,
			CL_GL_CONTEXT_KHR, (cl_context_properties)hRC,
			CL_WGL_HDC_KHR, (cl_context_properties)hDC,
			0
		};

		if (!clGetGLContextInfoKHR)
		{
			clGetGLContextInfoKHR = (clGetGLContextInfoKHR_fn)
				clGetExtensionFunctionAddressForPlatform(platform, "clGetGLContextInfoKHR");
			if (!clGetGLContextInfoKHR)
			{
				std::cout << "Failed to query proc address for clGetGLContextInfoKHR";
				return SDK_FAILURE;
			}
		}

		size_t deviceSize = 0;
		status = clGetGLContextInfoKHR(properties,
			CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR,
			0,
			NULL,
			&deviceSize);
		if (status != CL_SUCCESS)
		{
			std::cout << "clGetGLContextInfoKHR failed!!" << std::endl;
			return SDK_FAILURE;
		}

		if (deviceSize == 0)
		{
			// no interopable CL device found, cleanup
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(hRC);
			DeleteDC(hDC);
			hDC = NULL;
			hRC = NULL;
			DestroyWindow(gHwnd);
			// try the next display
			continue;
		}
		else
		{
			if (sampleArgs->deviceId == 0)
			{
				ShowWindow(gHwnd, SW_SHOW);
				//Found a winner
				break;
			}
			else if (sampleArgs->deviceId != connectedDisplays)
			{
				connectedDisplays++;
				wglMakeCurrent(NULL, NULL);
				wglDeleteContext(hRC);
				DeleteDC(hDC);
				hDC = NULL;
				hRC = NULL;
				DestroyWindow(gHwnd);
				if (xCoordinate >= 0)
				{
					xCoordinate1 += deviceMode.dmPelsWidth;
					// try the next display
				}
				else
				{
					xCoordinate1 -= deviceMode.dmPelsWidth;
				}

				continue;
			}
			else
			{
				ShowWindow(gHwnd, SW_SHOW);
				//Found a winner
				break;
			}
		}
	}

	if (!hRC || !hDC)
	{
		OPENCL_EXPECTED_ERROR("OpenGL interoperability is not feasible.");
	}

	cl_context_properties properties[] =
	{
		CL_CONTEXT_PLATFORM, (cl_context_properties)platform,
		CL_GL_CONTEXT_KHR, (cl_context_properties)hRC,
		CL_WGL_HDC_KHR, (cl_context_properties)hDC,
		0
	};

	if (sampleArgs->deviceType.compare("gpu") == 0)
	{
		status = clGetGLContextInfoKHR(properties,
			CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR,
			sizeof(cl_device_id),
			&interopDevice,
			NULL);
		CHECK_OPENCL_ERROR(status, "clGetGLContextInfoKHR failed!!");

		// Create OpenCL context from device's id
		context = clCreateContext(properties,
			1,
			&interopDevice,
			0,
			0,
			&status);
		CHECK_OPENCL_ERROR(status, "clCreateContext failed!!");
	}
	else
	{
		context = clCreateContextFromType(
			properties,
			CL_DEVICE_TYPE_CPU,
			NULL,
			NULL,
			&status);
		CHECK_OPENCL_ERROR(status, "clCreateContextFromType failed!!");
	}


	////////////// OpenGL Init //////////////
	openGL_Init_SA();
	////////////// OpenGL Init //////////////

}
#endif
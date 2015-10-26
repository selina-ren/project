//========= Copyright Valve Corporation ============//



#include <stdio.h>
#include <string>
#include <cstdlib>

#include <openvr.h>

#include "opencl_parts.hpp"

//#include "openvr_parts.hpp"

#include "lodepng.h"
#include "Matrices.h"
#include "pathtools.h"
#include "vr_function.hpp"




LightField_Render_GL *LightField_Render_GL::lightfield_Render = NULL;

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int main(int argc, char *argv[])
{

	LightField_Render_GL cllightfield_Render;
	LightField_Render_GL::lightfield_Render = &cllightfield_Render;

	if (cllightfield_Render.initialize() != SDK_SUCCESS)
	{
		return SDK_FAILURE;
	}

	//default file
	std::string viewer_config_file = "viewer_config.ini";
	std::string scene_config_file = "scene_config.ini";
	std::string refocusing_parameters_file = "refocusing_parameters.ini";


	for (int i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "viewer_config"))
		{
			viewer_config_file = argv[i + 1];
		}
		else if (!strcmp(argv[i], "scene_config"))
		{
			scene_config_file = argv[i + 1];
		}
		else if (!strcmp(argv[i], "refocus_para"))
		{
			refocusing_parameters_file = argv[i + 1];
		}
	}

	cllightfield_Render.loadViewerConfig(viewer_config_file);
	cllightfield_Render.loadAllScenefiles(scene_config_file);

	

	/*if (cllightfield_Render.using_HMD)
	{*/
		vr::IVRSystem *m_pHMD;
		vr::IVRCompositor *m_pCompositor;

		vr::HmdError eError = vr::HmdError_None;
		m_pHMD = vr::VR_Init(&eError);

		if (eError != vr::HmdError_None)
		{
			m_pHMD = NULL;
			char buf[1024];
			sprintf_s(buf, sizeof(buf), "Unable to init VR runtime: %s", vr::VR_GetStringForHmdError(eError));
			//SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "VR_Init Failed", buf, NULL);
			return false;
		}

		vr::HmdError peError = vr::HmdError_None;

		m_pCompositor = (vr::IVRCompositor*)vr::VR_GetGenericInterface(vr::IVRCompositor_Version, &peError);

		if (peError != vr::HmdError_None)
		{
			m_pCompositor = NULL;

			printf("Compositor initialization failed with error: %s\n", vr::VR_GetStringForHmdError(peError));
			return false;
		}

		uint32_t unSize = m_pCompositor->GetLastError(NULL, 0);
		if (unSize > 1)
		{
			char* buffer = new char[unSize];
			m_pCompositor->GetLastError(buffer, unSize);
			printf("Compositor - %s\n", buffer);
			delete[] buffer;
			return false;
		}

		

		uint32_t w, h;

		

		m_pHMD->GetRecommendedRenderTargetSize(&w, &h);

		printf("nview_res width is %d & height is %d", w, h);

		vr::HmdMatrix44_t matLeft = m_pHMD->GetProjectionMatrix(vr::Eye_Left, 0.0, 0.0, vr::API_OpenGL);
		vr::HmdMatrix44_t matRight = m_pHMD->GetProjectionMatrix(vr::Eye_Right, 0.0, 0.0, vr::API_OpenGL);

		vr::HmdMatrix34_t matEyeLeft = m_pHMD->GetEyeToHeadTransform(vr::Eye_Left);
		vr::HmdMatrix34_t matEyeRight = m_pHMD->GetEyeToHeadTransform(vr::Eye_Right);

		cllightfield_Render.matEyeToHeadLeft = GetHeadToEye(matEyeLeft);
		cllightfield_Render.matEyeToHeadRight = GetHeadToEye(matEyeRight);


		cllightfield_Render.nview_res_w = w;
		cllightfield_Render.nview_res_h = h;
		cllightfield_Render.nview_focal_length = 0.464894474;
		cllightfield_Render.nview_shear_left_hor = 0;
		cllightfield_Render.nview_shear_right_hor = 0;
		cllightfield_Render.nview_shear_left_ver =0;
		cllightfield_Render.nview_shear_right_ver = 0;

		cllightfield_Render.vrProjection_Shift();

		cllightfield_Render.setupCL();

		cllightfield_Render.run(m_pHMD, m_pCompositor);

		// Release All Data
		cllightfield_Render.cleanup();
		if (m_pHMD)
		{
			vr::VR_Shutdown();
			m_pHMD = NULL;
		}
	/*}
	else
	{

		cllightfield_Render.loadRefocusPara(refocusing_parameters_file);
		
		cllightfield_Render.setupCL_SA();
		cllightfield_Render.run_SA();
		cllightfield_Render.cleanup_SA();
	}*/



	return 0;
}

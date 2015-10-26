/**********************************************************************
Author: Yu Ji ©2015 
Project: Light Field Rendering on OpenCL


********************************************************************/

#ifndef SIMPLE_IMAGE_H_
#define SIMPLE_IMAGE_H_

#include "CommonDeclare.hpp"
#include <glm.hpp>

#define SAMPLE_VERSION "AMD-APP-SDK-v3.0.253.1"

#define LIGHT_FIELD_SRC "test_data.bmp"
#define OUTPUT_SAMPLE "output_sample.bmp"
#define LIGHT_FIELD_RENDER_OUTPUT "test_output.bmp"

#define GROUP_SIZE 256

#ifndef min
#define min(a, b)            (((a) < (b)) ? (a) : (b))
#endif

class LightField_Render_GL
{
	public:
		static LightField_Render_GL *lightfield_Render;
		cl_double setupTime;                /**< time taken to setup OpenCL resources and building kernel */
        cl_double kernelTime;               /**< time taken to run kernel and read result back */
        cl_uchar4* mapLFImageData;            /**< load bitmap data to device */
		cl_uchar4* mapPSFImageData;
        cl_uchar4* outputImageData;         /**< Output from device for 3D copy*/
        cl_uchar4* verificationImageData;      /**< Verify Output */

        cl_context context;                 /**< CL context */
        cl_device_id *devices;              /**< CL device list */

        cl_mem fillImage;                   /**< CL image buffer for fill Image*/
        cl_mem mapLFImage;                    /**< CL image buffer for map Image*/
        cl_mem outputImage;                 /**< CL image buffer for output Image*/
		cl_mem depth_map;
		cl_mem tempImage;
		cl_mem psfImage;

        cl_command_queue commandQueue;   /**< CL command queue */
        cl_program program;                 /**< CL program  */
        cl_event eventlist[2];              /**< CL event  */
        cl_event enqueueEvent;              /**< CL event  */

        cl_kernel kernelLightField_Render;            /**< CL kernel */
		cl_kernel kernelRefocusing;

        SDKBitMap mapLFBitmap;     /**< Bitmap class object */
		SDKBitMap mapPSFBitmap;
        
        uchar4* pixelData;       /**< Pointer to image data */
		uchar4* psfData;
		uchar4* outputpixelData;
        cl_uint pixelSize;                  /**< Size of a pixel in BMP format> */
        cl_uint width;                      /**< Width of image */
        cl_uint height;                     /**< Height of image */
		cl_uint psf_h;
		cl_uint psf_w;



		cl_int lfcam_res;					/**< image resolution of sub-aperture image */
		cl_int lfcam_num_height;			/**< image number in height */
		cl_int lfcam_num_width;				/**< image number in width */
		cl_float lfcam_fov;		/**< light field camera focal length */
		cl_int nview_res;
		cl_float nview_focal_length;
		cl_float4 nview_look_at;
		cl_float4 nview_up;
		cl_float4 nview_right;
		cl_float4 nview_pos;
		cl_float t_depth;
		cl_float depth_max;
		cl_float depth_min;

		cl_float nview_fl_max;
		cl_float nview_fl_min;

		cl_float d_focal;
		cl_float psf_R;
		cl_float psf_R_o;

		cl_float aper_min;
		cl_float aper_max;



        size_t blockSizeX;                  /**< Work-group size in x-direction */
        size_t blockSizeY;                  /**< Work-group size in y-direction */

        int iterations;                     /**< Number of iterations for kernel execution */
        
		clock_t t1, t2;
		int frameCount;
        int frameRefCount;
        double totalElapsedTime;

		cl_device_id interopDeviceId;


		GLuint pbo;                         //pixel-buffer object to hold-image data
        GLuint tex_fndisp;                         //Texture to display
		GLuint tex_depth;

		cl_bool imageSupport;               /**< Flag to check whether images are supported */
        cl_image_format imageFormat;        /**< Image format descriptor */
        cl_map_flags mapFlag;
        SDKDeviceInfo
        deviceInfo;                    /**< Structure to store device information*/
        KernelWorkGroupInfo
        kernelInfo;              /**< Structure to store kernel related info */

        SDKTimer    *sampleTimer;      /**< SDKTimer object */


	public:

		CLCommandArgs   *sampleArgs;   /**< CLCommand argument class */

		/**
        * Read bitmap image and allocate host memory
        * @param inputImageName name of the input file
		* @param outputImageName name of the output file
        * @return SDK_SUCCESS on success and SDK_FAILURE on failure
        */
		int readImage(std::string inputImageName, std::string psfImageName);


		/**
		* Read obj file and allocate host memory
		* @param objName name of the input file
		* @return SDK_SUCCESS on success and SDK_FAILURE on failure
		*/
		int readSceneGeo(std::string objName_scene, std::string objName_disp);


		/**
        * Read Light Field Slab configuration
        * @param configName name of the input file
        * @return SDK_SUCCESS on success and SDK_FAILURE on failure
        */
		int readConfig(std::string configName, std::string *mapLFImageName, std::string *objName, std::string * psfName);


		/**
        * Write to an image file
        * @param outputImageName name of the output file
        * @return SDK_SUCCESS on success and SDK_FAILURE on failure
        */
        int writeOutputImage(std::string outputImageName);

		/**
        * Constructor
        * Initialize member variables
        * @param name name of sample (string)
        */
        LightField_Render_GL()
            :
            mapLFImageData(NULL),
            outputImageData(NULL),
            verificationImageData(NULL)
        {
            pixelSize = sizeof(uchar4);
            pixelData = NULL;
			iterations = 1;
			blockSizeX = GROUP_SIZE;
            blockSizeY = 1;
            imageFormat.image_channel_data_type = CL_UNSIGNED_INT8;
            imageFormat.image_channel_order = CL_RGBA;
            
            mapFlag = CL_MAP_READ | CL_MAP_WRITE;

			//////
			frameCount = 0;
            frameRefCount = 90;
            totalElapsedTime = 0.0;

			nview_look_at.s0 = 0.0;
			nview_look_at.s1 = 0.0;
			nview_look_at.s2 = -1.0;
			nview_look_at.s3 = 0.0;
	
			nview_up.s0 = 0.0;
			nview_up.s1 = 1.0;
			nview_up.s2 = 0.0;
			nview_up.s3 = 0.0;

			nview_right.s0 = 1.0;
			nview_right.s1 = 0.0;
			nview_right.s2 = 0.0;
			nview_right.s3 = 0.0;

			nview_pos.s0 = 0.0;
			nview_pos.s1 = 0.0;
			nview_pos.s2 = 0.0;
			nview_pos.s3 = 0.0;


			//////


            sampleArgs = new CLCommandArgs() ;
            sampleTimer = new SDKTimer();
            sampleArgs->sampleVerStr = SAMPLE_VERSION;
        }

		~LightField_Render_GL()
        {
        }

		 /**
        * Allocate image memory and Load bitmap file
        * @return SDK_SUCCESS on success and SDK_FAILURE on failure
        */
        int setupLightField_Render();

		/**
         * Override from SDKSample, Generate binary image of given kernel
         * and exit application
         */
        int genBinaryImage();

        /**
        * OpenCL related initialisations.
        * Set up Context, Device list, Command Queue, Memory buffers
        * Build CL kernel program executable
        * @return SDK_SUCCESS on success and SDK_FAILURE on failure
        */
        int setupCL();

        /**
        * Set values for kernels' arguments, enqueue calls to the kernels
        * on to the command queue, wait till end of kernel execution.
        * Get kernel start and end time if timing is enabled
        * @return  SDK_SUCCESS on success and SDK_FAILURE on failure
        */
        int runCLKernels();

		/**
        * Reference CPU implementation of Binomial Option
        * for performance comparison
        */
        void LightField_RenderCPUReference();

		/**
        * Override from SDKSample. Print sample stats.
        */
		void printStats();

		/**
        * Initializing GL and get interoperable CL context
        * @param argc number of arguments
        * @param argv command line arguments
        * @
        * @return SDK_SUCCESS on success and SDK_FALIURE on failure.
        */
        int initializeGLAndGetCLContext(cl_platform_id platform,
                                        cl_context &context,
                                        cl_device_id &interopDevice);

        /**
        * Override from SDKSample. Initialize
        * command line parser, add custom options
        */
        int initialize();

        /**
        * Override from SDKSample, adjust width and height
        * of execution domain, perform all sample setup
        * @return  SDK_SUCCESS on success and SDK_FAILURE on failure
        */
        int setup();

        /**
        * Override from SDKSample
        * Run OpenCL ImageOverlap
        * @return  SDK_SUCCESS on success and SDK_FAILURE on failure
        */
        int run();

        /**
        * Override from SDKSample
        * Cleanup memory allocations
        * @return  SDK_SUCCESS on success and SDK_FAILURE on failure
        */
        int cleanup();

        /**
        * Override from SDKSample
        * Verify against reference implementation
        * @return  SDK_SUCCESS on success and SDK_FAILURE on failure
        */
        int verifyResults();

		/**
		* Rotate View Camera
		* input angle and rotate axis
		* @return  SDK_SUCCESS on success and SDK_FAILURE on failure
		*/
		int rotate_cam(float ang, int axis);


		/**
		* Rotate View Camera
		* input angle and rotate axis
		* @return  SDK_SUCCESS on success and SDK_FAILURE on failure
		*/
		int translate_cam(float ang, int axis);

		/**
		* keyboard action 
		* 
		*/
		void keybroad_action();


		/*depth version*/
		void render_fn_display();
		void render_geo();
		void setCamera(float* pos, float* look_dir, float* up_dir, float* viewMatrix);
		void setUniforms(float* projMatrix, float* viewMatrix, float Zfar_scale, int depthFlag);
		void convert_camera_pos_dir(float* cam_position, float* cam_direction, float* cam_up, float* cam_right);
		void calculate_cam_pos_dir();
		int openGL_Init();
		GLuint setupShaders();
		void setupBuffers_scene(std::vector<glm::vec3> vertices_geo, std::vector<glm::vec3> vertices_disp);
		void setupBuffers_display(std::vector<glm::vec3> vertices);
		void changeSize(int w, int h);
		void buildProjectionMatrix(float fov, float ratio, float nearP, float farP, float* projMatrix);

#ifdef _WIN32
        /**
         * enableGLAndGLContext
         * creates a GL Context on a specified device and get its deviceId
         * @param hWnd Window Handle
         * @param hRC context of window
         * @param platform cl_platform_id selected
         * @param context associated cl_context
         * @param interopDevice cl_device_id of selected device
         * @return SDK_SUCCESS on success and SDK_FAILURE on failure
         */
        int enableGLAndGetGLContext(HWND hWnd,
                                    HDC &hDC,
                                    HGLRC &hRC,
                                    cl_platform_id platform,
                                    cl_context &context,
                                    cl_device_id &interopDevice);

        void disableGL(HWND hWnd, HDC hDC, HGLRC hRC);
#endif
};

namespace appsdk
{


/**
* buildOpenCLProgram
* builds the opencl program
* @param program program object
* @param context cl_context object
* @param buildData buildProgramData Object
* @return 0 if success else nonzero
*/
int compileOpenCLProgram(cl_program &program, const cl_context& context,
                         buildProgramData &buildData);


}

#endif // SIMPLE_IMAGE_H_





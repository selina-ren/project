#pragma once

// openvr_driver.h
//========= Copyright Valve Corporation ============//
// Dynamically generated file. Do not modify this file directly.

#ifndef _OPENVR_DRIVER_API
#define _OPENVR_DRIVER_API

#include <stdint.h>


// vrtypes.h
#ifndef _INCLUDE_VRTYPES_H
#define _INCLUDE_VRTYPES_H 

namespace vr
{

#if defined(__linux__) || defined(__APPLE__) 
	// The 32-bit version of gcc has the alignment requirement for uint64 and double set to
	// 4 meaning that even with #pragma pack(8) these types will only be four-byte aligned.
	// The 64-bit version of gcc has the alignment requirement for these types set to
	// 8 meaning that unless we use #pragma pack(4) our structures will get bigger.
	// The 64-bit structure packing has to match the 32-bit structure packing for each platform.
	#pragma pack( push, 4 )
#else
	#pragma pack( push, 8 )
#endif

// right-handed system
// +y is up
// +x is to the right
// -z is going away from you
// Distance unit is  meters
struct HmdMatrix34_t
{
	float m[3][4];
};

struct HmdMatrix44_t
{
	float m[4][4];
};

struct HmdVector3_t
{
	float v[3];
};

struct HmdVector3d_t
{
	double v[3];
};

struct HmdVector2_t
{
	float v[2];
};

struct HmdQuaternion_t
{
	double w, x, y, z;
};

struct HmdQuad_t
{
	HmdVector3_t vCorners[ 4 ];
};

/** Used to return the post-distortion UVs for each color channel. 
* UVs range from 0 to 1 with 0,0 in the upper left corner of the 
* source render target. The 0,0 to 1,1 range covers a single eye. */
struct DistortionCoordinates_t
{
	float rfRed[2];
	float rfGreen[2];
	float rfBlue[2];
};

enum Hmd_Eye
{
	Eye_Left = 0,
	Eye_Right = 1
};

enum GraphicsAPIConvention
{
	API_DirectX = 0, // Normalized Z goes from 0 at the viewer to 1 at the far clip plane
	API_OpenGL = 1,  // Normalized Z goes from 1 at the viewer to -1 at the far clip plane
};

enum HmdTrackingResult
{
	TrackingResult_Uninitialized			= 1,

	TrackingResult_Calibrating_InProgress	= 100,
	TrackingResult_Calibrating_OutOfRange	= 101,

	TrackingResult_Running_OK				= 200,
	TrackingResult_Running_OutOfRange		= 201,
};

static const uint32_t k_unTrackingStringSize = 32;
static const uint32_t k_unMaxTrackedDeviceCount = 16;
static const uint32_t k_unTrackedDeviceIndex_Hmd = 0;
static const uint32_t k_unMaxDriverDebugResponseSize = 32768;

/** Describes what kind of object is being tracked at a given ID */
enum TrackedDeviceClass
{
	TrackedDeviceClass_Invalid = 0,				// the ID was not valid.
	TrackedDeviceClass_HMD = 1,					// Head-Mounted Displays
	TrackedDeviceClass_Controller = 2,			// Tracked controllers
	TrackedDeviceClass_TrackingReference = 4,	// Camera and base stations that serve as tracking reference points

	TrackedDeviceClass_Other = 1000,
};


/** describes a single pose for a tracked object */
struct TrackedDevicePose_t
{
	HmdMatrix34_t mDeviceToAbsoluteTracking;
	HmdVector3_t vVelocity;				// velocity in tracker space in m/s
	HmdVector3_t vAngularVelocity;		// angular velocity in radians/s (?)
	HmdTrackingResult eTrackingResult;
	bool bPoseIsValid;

	// This indicates that there is a device connected for this spot in the pose array.
	// It could go from true to false if the user unplugs the device.
	bool bDeviceIsConnected;
};

/** Identifies which style of tracking origin the application wants to use
* for the poses it is requesting */
enum TrackingUniverseOrigin
{
	TrackingUniverseSeated = 0,		// Poses are provided relative to the seated zero pose
	TrackingUniverseStanding = 1,	// Poses are provided relative to the safe bounds configured by the user
	TrackingUniverseRawAndUncalibrated = 2,	// Poses are provided in the coordinate system defined by the driver. You probably don't want this one.
};


/** Each entry in this enum represents a property that can be retrieved about a
* tracked device. Many fields are only valid for one TrackedDeviceClass. */
enum TrackedDeviceProperty
{
	// general properties that apply to all device classes
	Prop_TrackingSystemName_String			= 1000,
	Prop_ModelNumber_String					= 1001,
	Prop_SerialNumber_String				= 1002,
	Prop_RenderModelName_String				= 1003,
	Prop_WillDriftInYaw_Bool				= 1004,
	Prop_ManufacturerName_String			= 1005,
	Prop_TrackingFirmwareVersion_String		= 1006,
	Prop_HardwareRevision_String			= 1007,
	Prop_AllWirelessDongleDescriptions_String= 1008,
	Prop_ConnectedWirelessDongle_String		= 1009,
	Prop_DeviceIsWireless_Bool				= 1010,
	Prop_DeviceIsCharging_Bool				= 1011,
	Prop_DeviceBatteryPercentage_Float		= 1012, // 0 is empty, 1 is full
	Prop_StatusDisplayTransform_Matrix34	= 1013,

	// Properties that are unique to TrackedDeviceClass_HMD
	Prop_ReportsTimeSinceVSync_Bool			= 2000,
	Prop_SecondsFromVsyncToPhotons_Float	= 2001,
	Prop_DisplayFrequency_Float				= 2002,
	Prop_UserIpdMeters_Float				= 2003,
	Prop_CurrentUniverseId_Uint64			= 2004, 
	Prop_PreviousUniverseId_Uint64			= 2005, 
	Prop_DisplayFirmwareVersion_String		= 2006,
	Prop_IsOnDesktop_Bool					= 2007,

	// Properties that are unique to TrackedDeviceClass_Controller
	Prop_AttachedDeviceId_String			= 3000,
	Prop_SupportedButtons_Uint64			= 3001,
	Prop_Axis0Type_Int32					= 3002, // Return value is of type EVRControllerAxisType
	Prop_Axis1Type_Int32					= 3003, // Return value is of type EVRControllerAxisType
	Prop_Axis2Type_Int32					= 3004, // Return value is of type EVRControllerAxisType
	Prop_Axis3Type_Int32					= 3005, // Return value is of type EVRControllerAxisType
	Prop_Axis4Type_Int32					= 3006, // Return value is of type EVRControllerAxisType

	// Properties that are unique to TrackedDeviceClass_TrackingReference
	Prop_FieldOfViewLeftDegrees_Float		= 4000,
	Prop_FieldOfViewRightDegrees_Float		= 4001,
	Prop_FieldOfViewTopDegrees_Float		= 4002,
	Prop_FieldOfViewBottomDegrees_Float		= 4003,
	Prop_TrackingRangeMinimumMeters_Float	= 4004,
	Prop_TrackingRangeMaximumMeters_Float	= 4005,
};

/** Used to pass device IDs to API calls */
typedef uint32_t TrackedDeviceIndex_t;
static const uint32_t k_unTrackedDeviceIndexInvalid = 0xFFFFFFFF;

/** No string property will ever be longer than this length */
static const uint32_t k_unMaxPropertyStringSize = 32 * 1024;

/** Used to return errors that occur when reading properties. */
enum TrackedPropertyError
{
	TrackedProp_Success						= 0,
	TrackedProp_WrongDataType				= 1,
	TrackedProp_WrongDeviceClass			= 2,
	TrackedProp_BufferTooSmall				= 3,
	TrackedProp_UnknownProperty				= 4,
	TrackedProp_InvalidDevice				= 5,
	TrackedProp_CouldNotContactServer		= 6,
	TrackedProp_ValueNotProvidedByDevice	= 7,
	TrackedProp_StringExceedsMaximumLength	= 8,
};


/** a single vertex in a render model */
struct RenderModel_Vertex_t
{
	HmdVector3_t vPosition;		// position in meters in device space
	HmdVector3_t vNormal;		
	float rfTextureCoord[ 2 ];
};

/** A texture map for use on a render model */
struct RenderModel_TextureMap_t
{
	uint16_t unWidth, unHeight; // width and height of the texture map in pixels
	const uint8_t *rubTextureMapData;	// Map texture data. All textures are RGBA with 8 bits per channel per pixel. Data size is width * height * 4ub
};

/** Contains everything a game needs to render a single tracked or static object for the user. */
struct RenderModel_t
{
	uint64_t ulInternalHandle;					// Used internally by SteamVR
	const RenderModel_Vertex_t *rVertexData;			// Vertex data for the mesh
	uint32_t unVertexCount;						// Number of vertices in the vertex data
	const uint16_t *rIndexData;						// Indices into the vertex data for each triangle
	uint32_t unTriangleCount;					// Number of triangles in the mesh. Index count is 3 * TriangleCount
	RenderModel_TextureMap_t diffuseTexture;	// RGBA diffuse texture for the model
};


/** Allows the application to control what part of the provided texture will be used in the
* frame buffer. */
struct VRTextureBounds_t
{
	float uMin, vMin;
	float uMax, vMax;
};

/** The types of events that could be posted (and what the parameters mean for each event type) */
enum EVREventType
{
	VREvent_None = 0,

	VREvent_TrackedDeviceActivated		= 100,
	VREvent_TrackedDeviceDeactivated	= 101,
	VREvent_TrackedDeviceUpdated		= 102,

	VREvent_ButtonPress					= 200, // data is controller
	VREvent_ButtonUnpress				= 201, // data is controller
	VREvent_ButtonTouch					= 202, // data is controller
	VREvent_ButtonUntouch				= 203, // data is controller

	VREvent_MouseMove					= 300, // data is mouse
	VREvent_MouseButtonDown				= 301, // data is mouse
	VREvent_MouseButtonUp				= 302, // data is mouse

	VREvent_InputFocusCaptured			= 400, // data is process
	VREvent_InputFocusReleased			= 401, // data is process
	VREvent_SceneFocusLost				= 402, // data is process
	VREvent_SceneFocusGained			= 403, // data is process

	VREvent_OverlayShown				= 500,
	VREvent_OverlayHidden				= 501,
	VREvent_DashboardActivated		= 502,
	VREvent_DashboardDeactivated	= 503,
	VREvent_DashboardThumbSelected	= 504, // Sent to the overlay manager - data is overlay
	VREvent_DashboardRequested		= 505, // Sent to the overlay manager - data is overlay
	VREvent_ResetDashboard			= 506, // Send to the overlay manager

	VREvent_Notification_Show				= 600,
	VREvent_Notification_Dismissed			= 601,
	VREvent_Notification_BeginInteraction	= 602,

	VREvent_Quit						= 700, // data is process
	VREvent_ProcessQuit					= 701, // data is process

	VREvent_ChaperoneDataHasChanged		= 800,
};


/** VR controller button and axis IDs */
enum EVRButtonId
{
	k_EButton_System			= 0,
	k_EButton_ApplicationMenu	= 1,
	k_EButton_Grip				= 2,

	k_EButton_Axis0				= 32,
	k_EButton_Axis1				= 33,
	k_EButton_Axis2				= 34,
	k_EButton_Axis3				= 35,
	k_EButton_Axis4				= 36,

	// aliases for well known controllers
	k_EButton_SteamVR_Touchpad	= k_EButton_Axis0,
	k_EButton_SteamVR_Trigger	= k_EButton_Axis1,

	k_EButton_Max				= 64
};

inline uint64_t ButtonMaskFromId( EVRButtonId id ) { return 1ull << id; }

/** used for controller button events */
struct VREvent_Controller_t
{
	EVRButtonId button;
};


/** used for simulated mouse events in overlay space */
enum EVRMouseButton
{
	VRMouseButton_Left					= 0x0001,
	VRMouseButton_Right					= 0x0002,
	VRMouseButton_Middle				= 0x0004,
};


/** used for simulated mouse events in overlay space */
struct VREvent_Mouse_t
{
	float x, y;
	EVRMouseButton button;
};

/** notification related events. Details will still change at this point */
struct VREvent_Notification_t
{
	uint64_t ulUserValue;
	uint32_t notificationId;
};


/** Used for events about processes */
struct VREvent_Process_t
{
	uint32_t pid;
	uint32_t oldPid;
};


/** Used for a few events about overlays */
struct VREvent_Overlay_t
{
	uint64_t overlayHandle;
};


/** Not actually used for any events. It is just used to reserve
* space in the union for future event types */
struct VREvent_Reserved_t
{
	uint64_t reserved0;
	uint64_t reserved1;
};

/** If you change this you must manually update openvr_interop.cs.py */
typedef union
{
	VREvent_Reserved_t reserved;
	VREvent_Controller_t controller;
	VREvent_Mouse_t mouse;
	VREvent_Process_t process;
	VREvent_Notification_t notification;
	VREvent_Overlay_t overlay;
} VREvent_Data_t;

/** An event posted by the server to all running applications */
struct VREvent_t
{
	EVREventType eventType;
	TrackedDeviceIndex_t trackedDeviceIndex;
	VREvent_Data_t data;
	float eventAgeSeconds;
};


/** The mesh to draw into the stencil (or depth) buffer to perform 
* early stencil (or depth) kills of pixels that will never appear on the HMD.
* This mesh draws on all the pixels that will be hidden after distortion. 
*
* If the HMD does not provide a visible area mesh pVertexData will be
* NULL and unTriangleCount will be 0. */
struct HiddenAreaMesh_t
{
	const HmdVector2_t *pVertexData;
	uint32_t unTriangleCount;
};


/** Identifies what kind of axis is on the controller at index n. Read this type 
* with pVRSystem->Get( nControllerDeviceIndex, Prop_Axis0Type_Int32 + n );
*/
enum EVRControllerAxisType
{
	k_eControllerAxis_None = 0,
	k_eControllerAxis_TrackPad = 1,
	k_eControllerAxis_Joystick = 2,
	k_eControllerAxis_Trigger = 3, // Analog trigger data is in the X axis
};


/** contains information about one axis on the controller */
struct VRControllerAxis_t
{
	float x; // Ranges from -1.0 to 1.0 for joysticks and track pads. Ranges from 0.0 to 1.0 for triggers were 0 is fully released.
	float y; // Ranges from -1.0 to 1.0 for joysticks and track pads. Is always 0.0 for triggers.
};


/** the number of axes in the controller state */
static const uint32_t k_unControllerStateAxisCount = 5;


/** Holds all the state of a controller at one moment in time. */
struct VRControllerState001_t
{
	// If packet num matches that on your prior call, then the controller state hasn't been changed since 
	// your last call and there is no need to process it
	uint32_t unPacketNum;

	// bit flags for each of the buttons. Use ButtonMaskFromId to turn an ID into a mask
	uint64_t ulButtonPressed;
	uint64_t ulButtonTouched;

	// Axis data for the controller's analog inputs
	VRControllerAxis_t rAxis[ k_unControllerStateAxisCount ];
};


typedef VRControllerState001_t VRControllerState_t;


/** determines how to provide output to the application of various event processing functions. */
enum EVRControllerEventOutputType
{
	ControllerEventOutput_OSEvents = 0,
	ControllerEventOutput_VREvents = 1,
};


/** Allows the application to customize how the overlay appears in the compositor */
struct Compositor_OverlaySettings
{
	uint32_t size; // sizeof(Compositor_OverlaySettings)
	bool curved, antialias;
	float scale, distance, alpha;
	float uOffset, vOffset, uScale, vScale;
	float gridDivs, gridWidth, gridScale;
	HmdMatrix44_t transform;
};


/** error codes returned by Vr_Init */
enum HmdError
{
	HmdError_None = 0,
	HmdError_Unknown = 1,

	HmdError_Init_InstallationNotFound	= 100,
	HmdError_Init_InstallationCorrupt	= 101,
	HmdError_Init_VRClientDLLNotFound	= 102,
	HmdError_Init_FileNotFound			= 103,
	HmdError_Init_FactoryNotFound		= 104,
	HmdError_Init_InterfaceNotFound		= 105,
	HmdError_Init_InvalidInterface		= 106,
	HmdError_Init_UserConfigDirectoryInvalid = 107,
	HmdError_Init_HmdNotFound			= 108,
	HmdError_Init_NotInitialized		= 109,
	HmdError_Init_PathRegistryNotFound	= 110,
	HmdError_Init_NoConfigPath			= 111,
	HmdError_Init_NoLogPath				= 112,
	HmdError_Init_PathRegistryNotWritable = 113,

	HmdError_Driver_Failed				= 200,
	HmdError_Driver_Unknown				= 201,
	HmdError_Driver_HmdUnknown			= 202,
	HmdError_Driver_NotLoaded			= 203,
	HmdError_Driver_RuntimeOutOfDate	= 204,
	HmdError_Driver_HmdInUse			= 205,
	HmdError_Driver_NotCalibrated		= 206,
	HmdError_Driver_CalibrationInvalid	= 207,
	HmdError_Driver_HmdDisplayNotFound  = 208,

	HmdError_IPC_ServerInitFailed		= 300,
	HmdError_IPC_ConnectFailed			= 301,
	HmdError_IPC_SharedStateInitFailed	= 302,
	HmdError_IPC_CompositorInitFailed	= 303,
	HmdError_IPC_MutexInitFailed		= 304,

	HmdError_VendorSpecific_UnableToConnectToOculusRuntime = 1000,

	HmdError_Steam_SteamInstallationNotFound = 2000,

};

#pragma pack( pop )

// figure out how to import from the VR API dll
#if defined(_WIN32)

#ifdef VR_API_EXPORT
#define VR_INTERFACE extern "C" __declspec( dllexport )
#else
#define VR_INTERFACE extern "C" __declspec( dllimport )
#endif

#elif defined(GNUC) || defined(COMPILER_GCC) || defined(__APPLE__)

#ifdef VR_API_EXPORT
#define VR_INTERFACE extern "C" __attribute__((visibility("default")))
#else
#define VR_INTERFACE extern "C" 
#endif

#else
#error "Unsupported Platform."
#endif


#if defined( _WIN32 )
#define VR_CALLTYPE __cdecl
#else
#define VR_CALLTYPE 
#endif

}

#endif // _INCLUDE_VRTYPES_H

// iservertrackeddevicedriver.h
namespace vr
{

/** describes the high level state of the tracked object */
struct TrackedDeviceDriverInfo_t
{
	char rchTrackingSystemId[ k_unTrackingStringSize ];	// Name of the underlying tracking system
	char rchSerialNumber[ k_unTrackingStringSize ];		// Serial number of the tracked object
	char rchModelNumber[ k_unTrackingStringSize ];		// Model number of the tracked object
	char rchRenderModelName[ k_unTrackingStringSize ];	// Pass this to GetRenderModel to get the mesh and texture to render this device

	TrackedDeviceClass eClass;	

	// This indicates that there is a device connected for this spot in the info array.
	// It could go from true to false if the user unplugs the device.
	bool bDeviceIsConnected;

	// This will be true for gyro-only tracking systems
	// with no ground truth.
	bool bWillDriftInYaw;

	// ---- HMD capabilities ----

	// This is true if the device (or SteamVR's software layer) supports reporting precise 
	// times for vsync
	bool bReportsTimeSinceVSync;

	// The number of seconds that pass between when the video card sends vsync
	// and when photons hit the wearer's eyes. Use this with GetSecondsSinceLastVsync()
	// to figure out what to pass to GetTrackerFromDevicePose()
	float fSecondsFromVsyncToPhotons;

	// The number of frames per second on the display itself. Applications should
	// target this frame rate to keep up with the display
	float fDisplayFrequency;
};



struct DriverPoseQuaternion_t
{
	double w, x, y, z;
};

struct DriverPose_t
{
	/* Time offset of this pose, in seconds from the actual time of the pose,
	 * relative to the time of the PoseUpdated() call made by the driver.
	 */
	double poseTimeOffset;

	/* Generally, the pose maintained by a driver
	 * is in an inertial coordinate system different
	 * from the world system of x+ right, y+ up, z+ back.
	 * Also, the driver is not usually tracking the "head" position,
	 * but instead an internal IMU or another reference point in the HMD.
	 * The following two transforms transform positions and orientations
	 * to app world space from driver world space,
	 * and to HMD head space from driver local body space. 
	 *
	 * We maintain the driver pose state in its internal coordinate system,
	 * so we can do the pose prediction math without having to
	 * use angular acceleration.  A driver's angular acceleration is generally not measured,
	 * and is instead calculated from successive samples of angular velocity.
	 * This leads to a noisy angular acceleration values, which are also
	 * lagged due to the filtering required to reduce noise to an acceptable level.
	 */
	vr::HmdQuaternion_t qWorldFromDriverRotation;
	double vecWorldFromDriverTranslation[ 3 ];

	vr::HmdQuaternion_t qDriverFromHeadRotation;
	double vecDriverFromHeadTranslation[ 3 ];

	/* State of driver pose, in meters and radians. */
	/* Position of the driver tracking reference in driver world space
	* +[0] (x) is right
	* +[1] (y) is up
	* -[2] (z) is forward
	*/
	double vecPosition[ 3 ];

	/* Velocity of the pose in meters/second */
	double vecVelocity[ 3 ];

	/* Acceleration of the pose in meters/second */
	double vecAcceleration[ 3 ];

	/* Orientation of the tracker, represented as a quaternion */
	vr::HmdQuaternion_t qRotation;

	/* Angular velocity of the pose in axis-angle 
	* representation. The direction is the angle of
	* rotation and the magnitude is the angle around
	* that axis in radians/second. */
	double vecAngularVelocity[ 3 ];

	/* Angular acceleration of the pose in axis-angle 
	* representation. The direction is the angle of
	* rotation and the magnitude is the angle around
	* that axis in radians/second^2. */
	double vecAngularAcceleration[ 3 ];

	HmdTrackingResult result;

	bool poseIsValid;
	bool willDriftInYaw;
	bool shouldApplyHeadModel;
};


// ----------------------------------------------------------------------------------------------
// Purpose: Represents a single tracked device in a driver
// ----------------------------------------------------------------------------------------------
class ITrackedDeviceServerDriver
{
public:

	// ------------------------------------
	// Management Methods
	// ------------------------------------
	/** This is called before an HMD is returned to the application. It will always be
	* called before any display or tracking methods. Memory and processor use by the
	* ITrackedDeviceServerDriver object should be kept to a minimum until it is activated. 
	* The pose listener is guaranteed to be valid until Deactivate is called, but 
	* should not be used after that point. */
	virtual HmdError Activate( uint32_t unObjectId ) = 0;

	/** This is called when The VR system is switching from this Hmd being the active display
	* to another Hmd being the active display. The driver should clean whatever memory
	* and thread use it can when it is deactivated */
	virtual void Deactivate() = 0;

	/** returns the ID of this particular HMD. This value is opaque to the VR system itself,
	* but should be unique within the driver because it will be passed back in via FindHmd */
	virtual const char *GetId() = 0;

	/** A VR Client has made this debug request of the driver. The set of valid requests is entirely
	* up to the driver and the client to figure out, as is the format of the response. Responses that
	* exceed the length of the supplied buffer should be truncated and null terminated */
	virtual void DebugRequest( const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize ) = 0;

	// ------------------------------------
	// Display Methods
	// ------------------------------------

	/** Size and position that the window needs to be on the VR display. */
	virtual void GetWindowBounds( int32_t *pnX, int32_t *pnY, uint32_t *pnWidth, uint32_t *pnHeight ) = 0;

	/** Returns true if the display is extending the desktop. */
	virtual bool IsDisplayOnDesktop() = 0;

	/** Returns true if the display is real and not a fictional display. */
	virtual bool IsDisplayRealDisplay() = 0;

	/** Suggested size for the intermediate render target that the distortion pulls from. */
	virtual void GetRecommendedRenderTargetSize( uint32_t *pnWidth, uint32_t *pnHeight ) = 0;

	/** Gets the viewport in the frame buffer to draw the output of the distortion into */
	virtual void GetEyeOutputViewport( Hmd_Eye eEye, uint32_t *pnX, uint32_t *pnY, uint32_t *pnWidth, uint32_t *pnHeight ) = 0;

	/** The components necessary to build your own projection matrix in case your
	* application is doing something fancy like infinite Z */
	virtual void GetProjectionRaw( Hmd_Eye eEye, float *pfLeft, float *pfRight, float *pfTop, float *pfBottom ) = 0;

	/** Returns the transform from eye space to the head space. Eye space is the per-eye flavor of head
	* space that provides stereo disparity. Instead of Model * View * Projection the sequence is Model * View * Eye^-1 * Projection. 
	* Normally View and Eye^-1 will be multiplied together and treated as View in your application. 
	*/
	virtual HmdMatrix34_t GetHeadFromEyePose( Hmd_Eye eEye ) = 0;

	/** Returns the result of the distortion function for the specified eye and input UVs. UVs go from 0,0 in 
	* the upper left of that eye's viewport and 1,1 in the lower right of that eye's viewport. */
	virtual DistortionCoordinates_t ComputeDistortion( Hmd_Eye eEye, float fU, float fV ) = 0;

	// -----------------------------------
	// Assorted capability methods
	// -----------------------------------

	/** Returns all the static information that will be reported to the clients */
	virtual TrackedDeviceDriverInfo_t GetTrackedDeviceDriverInfo() = 0;
	
	// -----------------------------------
	// Administrative Methods
	// -----------------------------------

	/** Returns the model number of this HMD */
	virtual const char *GetModelNumber() = 0;

	/** Returns the serial number of this HMD */
	virtual const char *GetSerialNumber() = 0;

	// ------------------------------------
	// IPD Methods
	// ------------------------------------

	/** Gets the current IPD (Interpupillary Distance) in meters. */
	virtual float GetIPD() = 0;

	// ------------------------------------
	// Tracking Methods
	// ------------------------------------
	virtual DriverPose_t GetPose() = 0;


	// ------------------------------------
	// Property Methods
	// ------------------------------------

	/** Returns a bool property. If the property is not available this function will return false. */
	virtual bool GetBoolTrackedDeviceProperty( TrackedDeviceProperty prop, TrackedPropertyError *pError ) = 0;

	/** Returns a float property. If the property is not available this function will return 0. */
	virtual float GetFloatTrackedDeviceProperty( TrackedDeviceProperty prop, TrackedPropertyError *pError ) = 0;

	/** Returns an int property. If the property is not available this function will return 0. */
	virtual int32_t GetInt32TrackedDeviceProperty( TrackedDeviceProperty prop, TrackedPropertyError *pError ) = 0;

	/** Returns a uint64 property. If the property is not available this function will return 0. */
	virtual uint64_t GetUint64TrackedDeviceProperty( TrackedDeviceProperty prop, TrackedPropertyError *pError ) = 0;

	/** Returns a matrix property. If the device index is not valid or the property is not a matrix type, this function will return identity. */
	virtual HmdMatrix34_t GetMatrix34TrackedDeviceProperty( TrackedDeviceProperty prop, TrackedPropertyError *pError ) = 0;

	/** Returns a string property. If the property is not available this function will return 0 and pError will be 
	* set to an error. Otherwise it returns the length of the number of bytes necessary to hold this string including 
	* the trailing null. If the buffer is too small the error will be TrackedProp_BufferTooSmall. Strings will 
	* generally fit in buffers of k_unTrackingStringSize characters. Drivers may not return strings longer than 
	* k_unMaxPropertyStringSize. */
	virtual uint32_t GetStringTrackedDeviceProperty( TrackedDeviceProperty prop, char *pchValue, uint32_t unBufferSize, TrackedPropertyError *pError ) = 0;

	// ------------------------------------
	// Controller Methods
	// ------------------------------------

	/** Gets the current state of a controller. */
	virtual VRControllerState_t GetControllerState() = 0;

	/** Returns a uint64 property. If the property is not available this function will return 0. */
	virtual bool TriggerHapticPulse( uint32_t unAxisId, uint16_t usPulseDurationMicroseconds ) = 0;
	
};



static const char *ITrackedDeviceServerDriver_Version = "ITrackedDeviceServerDriver_001";

}// itrackeddevicedriverprovider.h
namespace vr
{

class ITrackedDeviceServerDriver;
struct TrackedDeviceDriverInfo_t;
struct DriverPose_t;

class IDriverLog
{
public:
	/** Writes a log message to the log file prefixed with the driver name */
	virtual void Log( const char *pchLogMessage ) = 0;
};

/** This interface is provided by vrserver to allow the driver to notify 
* the system when something changes about a device. These changes must
* not change the serial number or class of the device because those values
* are permanently associated with the device's index. */
class IServerDriverHost
{
public:
	/** Notifies the server that a tracked device has been added. If this function returns true
	* the server will call Activate on the device. If it returns false some kind of error
	* has occurred and the device will not be activated. */
	virtual bool TrackedDeviceAdded( const TrackedDeviceDriverInfo_t & info ) = 0;

	/** Notifies the server that a tracked device info has changed. These changes must not
	* change the serial number or class of the device because those values are permanently associated with the device's 
	* index. */
	virtual void TrackedDeviceInfoUpdated( uint32_t unWhichDevice, const TrackedDeviceDriverInfo_t & info ) = 0;

	/** Notifies the server that a tracked device's pose has been updated */
	virtual void TrackedDevicePoseUpdated( uint32_t unWhichDevice, const DriverPose_t & newPose ) = 0;

	/** Notifies the server that the property cache for the specified device should be invalidated */
	virtual void TrackedDevicePropertiesChanged( uint32_t unWhichDevice ) = 0;

	/** Notifies the server that vsync has occurred on the the display attached to the device. This is
	* only permitted on devices of the HMD class. */
	virtual void VsyncEvent( double vsyncTimeOffsetSeconds ) = 0;

	/** notifies the server that the button was pressed */
	virtual void TrackedDeviceButtonPressed( uint32_t unWhichDevice, EVRButtonId eButtonId, double eventTimeOffset ) = 0;

	/** notifies the server that the button was unpressed */
	virtual void TrackedDeviceButtonUnpressed( uint32_t unWhichDevice, EVRButtonId eButtonId, double eventTimeOffset ) = 0;

	/** notifies the server that the button was pressed */
	virtual void TrackedDeviceButtonTouched( uint32_t unWhichDevice, EVRButtonId eButtonId, double eventTimeOffset ) = 0;

	/** notifies the server that the button was unpressed */
	virtual void TrackedDeviceButtonUntouched( uint32_t unWhichDevice, EVRButtonId eButtonId, double eventTimeOffset ) = 0;

	/** notifies the server than a controller axis changed */
	virtual void TrackedDeviceAxisUpdated( uint32_t unWhichDevice, uint32_t unWhichAxis, const VRControllerAxis_t & axisState ) = 0;
};


/** This interface must be implemented in each driver. It will be loaded in vrserver.exe */
class IServerTrackedDeviceProvider
{
public:
	/** initializes the driver. This will be called before any other methods are called.
	* If Init returns anything other than HmdError_None the driver DLL will be unloaded.
	*
	* pchUserDriverConfigDir - The absolute path of the directory where the driver should store user
	*	config files.
	* pchDriverInstallDir - The absolute path of the root directory for the driver.
	*/
	virtual HmdError Init( IDriverLog *pDriverLog, vr::IServerDriverHost *pDriverHost, const char *pchUserDriverConfigDir, const char *pchDriverInstallDir ) = 0;

	/** cleans up the driver right before it is unloaded */
	virtual void Cleanup() = 0;

	/** returns the number of HMDs that this driver manages that are physically connected. */
	virtual uint32_t GetTrackedDeviceCount() = 0;

	/** returns a single HMD */
	virtual ITrackedDeviceServerDriver *GetTrackedDeviceDriver( uint32_t unWhich ) = 0;

	/** returns a single HMD by ID */
	virtual ITrackedDeviceServerDriver* FindTrackedDeviceDriver( const char *pchId ) = 0;

	/** Allows the driver do to some work in the main loop of the server. */
	virtual void RunFrame() = 0;

};


static const char *IServerTrackedDeviceProvider_Version = "IServerTrackedDeviceProvider_001";


/** This interface is provided by vrclient to allow the driver call back and query various information */
class IClientDriverHost
{
public:
	/** Returns the device class of a tracked device. If there has not been a device connected in this slot
	* since the application started this function will return TrackedDevice_Invalid. For previous detected
	* devices the function will return the previously observed device class. 
	*
	* To determine which devices exist on the system, just loop from 0 to k_unMaxTrackedDeviceCount and check
	* the device class. Every device with something other than TrackedDevice_Invalid is associated with an 
	* actual tracked device. */
	virtual TrackedDeviceClass GetTrackedDeviceClass( vr::TrackedDeviceIndex_t unDeviceIndex ) = 0;

	/** Returns true if there is a device connected in this slot. */
	virtual bool IsTrackedDeviceConnected( vr::TrackedDeviceIndex_t unDeviceIndex ) = 0;

	/** Returns a bool property. If the device index is not valid or the property is not a bool type this function will return false. */
	virtual bool GetBoolTrackedDeviceProperty( vr::TrackedDeviceIndex_t unDeviceIndex, TrackedDeviceProperty prop, TrackedPropertyError *pError = 0L ) = 0;

	/** Returns a float property. If the device index is not valid or the property is not a float type this function will return 0. */
	virtual float GetFloatTrackedDeviceProperty( vr::TrackedDeviceIndex_t unDeviceIndex, TrackedDeviceProperty prop, TrackedPropertyError *pError = 0L ) = 0;

	/** Returns an int property. If the device index is not valid or the property is not a int type this function will return 0. */
	virtual int32_t GetInt32TrackedDeviceProperty( vr::TrackedDeviceIndex_t unDeviceIndex, TrackedDeviceProperty prop, TrackedPropertyError *pError = 0L ) = 0;

	/** Returns a uint64 property. If the device index is not valid or the property is not a uint64 type this function will return 0. */
	virtual uint64_t GetUint64TrackedDeviceProperty( vr::TrackedDeviceIndex_t unDeviceIndex, TrackedDeviceProperty prop, TrackedPropertyError *pError = 0L ) = 0;

	/** Returns a string property. If the device index is not valid or the property is not a float type this function will 
	* return 0. Otherwise it returns the length of the number of bytes necessary to hold this string including the trailing
	* null. Strings will generally fit in buffers of k_unTrackingStringSize characters. */
	virtual uint32_t GetStringTrackedDeviceProperty( vr::TrackedDeviceIndex_t unDeviceIndex, TrackedDeviceProperty prop, char *pchValue, uint32_t unBufferSize, TrackedPropertyError *pError = 0L ) = 0;

};



/** This interface must be implemented in each driver. It will be loaded in vrclient.dll */
class IClientTrackedDeviceProvider
{
public:
	/** initializes the driver. This will be called before any other methods are called,
	* except BIsHmdPresent(). BIsHmdPresent is called outside of the Init/Cleanup pair.
	* If Init returns anything other than HmdError_None the driver DLL will be unloaded.
	*
	* pchUserDriverConfigDir - The absolute path of the directory where the driver should store user
	*	config files.
	* pchDriverInstallDir - The absolute path of the root directory for the driver.
	*/
	virtual HmdError Init( IDriverLog *pDriverLog, vr::IClientDriverHost *pDriverHost, const char *pchUserDriverConfigDir, const char *pchDriverInstallDir ) = 0;

	/** cleans up the driver right before it is unloaded */
	virtual void Cleanup() = 0;

	/** Called when the client needs to inform an application if an HMD is attached that uses
	* this driver. This method should be as lightweight as possible and should have no side effects
	* such as hooking process functions or leaving resources loaded. Init will not be called before 
	* this method and Cleanup will not be called after it.
	*/
	virtual bool BIsHmdPresent( const char *pchUserConfigDir ) = 0;

	/** called when the client inits an HMD to let the client driver know which one is in use */
	virtual HmdError SetDisplayId( const char *pchDisplayId ) = 0;

	/** [Windows Only]
	* Notifies the driver that the VR output will appear in a particular window.
	*/
	virtual bool AttachToWindow( void *hWnd ) = 0;

	/** Returns the stencil mesh information for the current HMD. If this HMD does not have a stencil mesh the vertex data and count will be
	* NULL and 0 respectively. This mesh is meant to be rendered into the stencil buffer (or into the depth buffer setting nearz) before rendering
	* each eye's view. The pixels covered by this mesh will never be seen by the user after the lens distortion is applied and based on visibility to the panels.
	* This will improve perf by letting the GPU early-reject pixels the user will never see before running the pixel shader.
	* NOTE: Render this mesh with backface culling disabled since the winding order of the vertices can be different per-HMD or per-eye.
	*/
	virtual HiddenAreaMesh_t GetHiddenAreaMesh( Hmd_Eye eEye ) = 0;
};

static const char *IClientTrackedDeviceProvider_Version = "IClientTrackedDeviceProvider_001";

}// End

#endif // _OPENVR_DRIVER_API




#include "vr_function.hpp"




Matrix4 GetCurrentViewMatrix(Matrix4 m_mat4eyePos, Matrix4 m_mat4HMDPose)
{
	Matrix4 matMVP;
	
		matMVP = m_mat4eyePos * m_mat4HMDPose;
	
		

	return matMVP;
}

Matrix4 ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t &matPose)
{
	Matrix4 matrixObj(
		matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0,
		matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0,
		matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0,
		matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f

		/*matPose.m[0][0], matPose.m[0][1], matPose.m[0][2], 0.0,
		matPose.m[1][0], matPose.m[1][1], matPose.m[1][2], 0.0,
		matPose.m[2][0], matPose.m[2][1], matPose.m[2][2], 0.0,
		matPose.m[3][0], matPose.m[3][1], matPose.m[3][2], 1.0f*/
		);
	return matrixObj;
}

Matrix4 GetHeadToEye(vr::HmdMatrix34_t &matPose)
{
	Matrix4 matrixObj(
		matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0,
		matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0,
		matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0,
		matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f
		);

	return matrixObj.invert();
}



Matrix4 UpdateHMDMatrixPose(vr::IVRSystem *m_pHMD, vr::IVRCompositor *m_pCompositor)
{
	Matrix4 m_mat4HMDPose;
	int m_iValidPoseCount;
	std::string m_strPoseClasses;

	vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
	Matrix4 m_rmat4DevicePose[vr::k_unMaxTrackedDeviceCount];
	char m_rDevClassChar[vr::k_unMaxTrackedDeviceCount];

	if (m_pCompositor)
	{
		m_pCompositor->WaitGetPoses(m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0);
	}
	else
	{
		// We just got done with the glFinish - the seconds since last vsync should be 0.
		float fSecondsSinceLastVsync = 0.0f;

		float fFrameDuration = 1.0f / m_pHMD->GetFloatTrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_DisplayFrequency_Float);

		float fSecondsUntilPhotons = fFrameDuration - fSecondsSinceLastVsync + m_pHMD->GetFloatTrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SecondsFromVsyncToPhotons_Float);
		m_pHMD->GetDeviceToAbsoluteTrackingPose(vr::TrackingUniverseStanding, fSecondsUntilPhotons, m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount);
	}

	m_iValidPoseCount = 0;
	m_strPoseClasses = "";
	for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice)
	{
		if (m_rTrackedDevicePose[nDevice].bPoseIsValid)
		{
			m_iValidPoseCount++;
			m_rmat4DevicePose[nDevice] = ConvertSteamVRMatrixToMatrix4(m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking);
			if (m_rDevClassChar[nDevice] == 0)
			{
				switch (m_pHMD->GetTrackedDeviceClass(nDevice))
				{
				case vr::TrackedDeviceClass_Controller:        m_rDevClassChar[nDevice] = 'C'; break;
				case vr::TrackedDeviceClass_HMD:               m_rDevClassChar[nDevice] = 'H'; break;
				case vr::TrackedDeviceClass_Invalid:           m_rDevClassChar[nDevice] = 'I'; break;
				case vr::TrackedDeviceClass_Other:             m_rDevClassChar[nDevice] = 'O'; break;
				case vr::TrackedDeviceClass_TrackingReference: m_rDevClassChar[nDevice] = 'T'; break;
				default:                                       m_rDevClassChar[nDevice] = '?'; break;
				}
			}
			m_strPoseClasses += m_rDevClassChar[nDevice];
		}
	}

	if (m_rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid)
	{
		m_mat4HMDPose = m_rmat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd].invert();
	}

	return m_mat4HMDPose;

}



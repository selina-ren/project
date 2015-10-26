#include <openvr.h>
#include "Matrices.h"










Matrix4 GetCurrentViewMatrix(Matrix4 m_mat4eyePos, Matrix4 m_mat4HMDPose);

Matrix4 GetHeadToEye(vr::HmdMatrix34_t &matPose);

Matrix4 UpdateHMDMatrixPose(vr::IVRSystem *m_pHMD, vr::IVRCompositor *m_pCompositor);

Matrix4 ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t &matPose);
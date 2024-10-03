/*
William Duprey
10/3/24
Camera Implementation
*/

#include "Camera.h"
#include "Input.h"
using namespace DirectX;

Camera::Camera(
    XMFLOAT3 startPosition, 
    float _aspectRatio, 
    float _fov, 
    float _nearClip, 
    float _farClip, 
    bool _doPerspective) :
        aspectRatio(_aspectRatio),
        fov(_fov),
        nearClip(_nearClip),
        farClip(_farClip),
        doPerspective(_doPerspective)
{
    // Create transform and set starting position
    transform = std::make_shared<Transform>();
    transform->SetPosition(startPosition);

    // Calculate view and projection matrices
    UpdateViewMatrix();
    UpdateProjectionMatrix(aspectRatio);
}

Camera::~Camera()
{

}

void Camera::Update(float dt)
{

}

void Camera::UpdateViewMatrix()
{

}

void Camera::UpdateProjectionMatrix(float aspectRatio)
{

}


///////////////////////////////////////////////////////////////////////////////
// ------------------------------- GETTERS --------------------------------- //
///////////////////////////////////////////////////////////////////////////////
XMFLOAT4X4 Camera::GetViewMatrix()
{
    return DirectX::XMFLOAT4X4();
}

XMFLOAT4X4 Camera::GetProjectionMatrix()
{
    return DirectX::XMFLOAT4X4();
}

std::shared_ptr<Transform> Camera::GetTransform()
{
    return std::shared_ptr<Transform>();
}

float Camera::GetAspectRatio()
{
    return 0.0f;
}

float Camera::GetFieldOfView()
{
    return 0.0f;
}

float Camera::GetNearClip()
{
    return 0.0f;
}

float Camera::GetFarClip()
{
    return 0.0f;
}

float Camera::GetMoveSpeed()
{
    return 0.0f;
}

float Camera::GetLookSpeed()
{
    return 0.0f;
}

bool Camera::GetProjectionType()
{
    return false;
}


///////////////////////////////////////////////////////////////////////////////
// ------------------------------- SETTERS --------------------------------- //
///////////////////////////////////////////////////////////////////////////////
void Camera::SetFieldOfView(float _fov)
{

}

void Camera::SetNearClip(float _nearClip)
{

}

void Camera::SetFarClip(float _farClip)
{

}

void Camera::SetMoveSpeed(float _moveSpeed)
{

}

void Camera::SetLookSpeed(float _lookSpeed)
{

}

void Camera::SetProjectionType(bool _doPerspective)
{

}

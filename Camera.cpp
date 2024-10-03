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
    bool _doPerspective,
    float _orthoWidth,
    float _moveSpeed,
    float _lookSpeed) :
        aspectRatio(_aspectRatio),
        fov(_fov),
        nearClip(_nearClip),
        farClip(_farClip),
        doPerspective(_doPerspective),
        orthoWidth(_orthoWidth),
        moveSpeed(_moveSpeed),
        lookSpeed(_lookSpeed)
{
    // Create transform and set starting position
    transform = std::make_shared<Transform>();
    transform->SetPosition(startPosition);

    // Calculate view and projection matrices
    UpdateViewMatrix();
    UpdateProjectionMatrix(aspectRatio);
}

// Shared pointers clean themselves up
Camera::~Camera() { }


///////////////////////////////////////////////////////////////////////////////
// ------------------------------- UPDATE ---------------------------------- //
///////////////////////////////////////////////////////////////////////////////
void Camera::Update(float dt)
{

}

// --------------------------------------------------------
// Calculates the camera's view matrix.
// Called when created, and once per frame, 
// as part of the camera Update method.
// --------------------------------------------------------
void Camera::UpdateViewMatrix()
{
    // These need to be variables and not just return values
    XMFLOAT3 pos = transform->GetPosition();
    XMFLOAT3 forward = transform->GetForward();

    // Create and save the view matrix
    XMStoreFloat4x4(&viewMatrix, 
        XMMatrixLookToLH(
            XMLoadFloat3(&pos),         // Camera position
            XMLoadFloat3(&forward),     // Camera forward direction
            XMVectorSet(0, 1, 0, 0)));  // World up direction
}

// --------------------------------------------------------
// Calculates the camera's projection matrix.
// Called when created, and whenever the window is resized.
// --------------------------------------------------------
void Camera::UpdateProjectionMatrix(float _aspectRatio)
{
    // Set the new aspect ratio provided as the parameter
    aspectRatio = _aspectRatio;

    // Stores the result of the projection,
    // either perspective or orthographic
    XMMATRIX proj;

    // If doPerspective is true, do... perspective
    if (doPerspective)
    {
        proj = XMMatrixPerspectiveFovLH(
            fov, aspectRatio, nearClip, farClip);
    }
    // Otherwise, do orthographic
    else 
    {
        // No need to store the view height, since it can
        // be derived from the view width and the aspect ratio
        proj = XMMatrixOrthographicLH(
            orthoWidth, orthoWidth / aspectRatio, nearClip, farClip);
    }
    
    // Store the result back in the projMatrix field
    XMStoreFloat4x4(&projMatrix, proj);
}


///////////////////////////////////////////////////////////////////////////////
// ------------------------------- GETTERS --------------------------------- //
///////////////////////////////////////////////////////////////////////////////
XMFLOAT4X4 Camera::GetViewMatrix() { return viewMatrix; }
XMFLOAT4X4 Camera::GetProjectionMatrix() { return projMatrix; }
std::shared_ptr<Transform> Camera::GetTransform() { return transform; }
float Camera::GetAspectRatio() { return aspectRatio; }
float Camera::GetFieldOfView() { return fov; }
float Camera::GetNearClip() { return nearClip; }
float Camera::GetFarClip() { return farClip; }
float Camera::GetMoveSpeed() { return moveSpeed; }
float Camera::GetLookSpeed() { return lookSpeed; }
bool Camera::GetProjectionType() { return doPerspective; }


///////////////////////////////////////////////////////////////////////////////
// ------------------------------- SETTERS --------------------------------- //
///////////////////////////////////////////////////////////////////////////////
void Camera::SetFieldOfView(float _fov) { fov = _fov; }
void Camera::SetNearClip(float _nearClip) { nearClip = _nearClip; }
void Camera::SetFarClip(float _farClip) { farClip = _farClip; }
void Camera::SetMoveSpeed(float _moveSpeed) { moveSpeed = _moveSpeed; }
void Camera::SetLookSpeed(float _lookSpeed) { lookSpeed = _lookSpeed; }
void Camera::SetProjectionType(bool _doPerspective) { doPerspective = _doPerspective; }

/*
William Duprey
9/25/24
Transform Class Implementation
*/

#include "Transform.h"
using namespace DirectX;

// --------------------------------------------------------
// Constructor for a transform object. Initializes its
// fields to line up with an identity matrix for the world.
// --------------------------------------------------------
Transform::Transform() : 
    position(0, 0, 0),
    rotation(0, 0, 0),
    scale(1, 1, 1),
    dirtyWorld(false),
    worldMatrix(1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1),
    worldInverseTransposeMatrix(worldMatrix)
{
    // Better to do it this way or by using initializer list?
    //XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
    //XMStoreFloat4x4(&worldInverseTransposeMatrix, XMMatrixIdentity());
}


///////////////////////////////////////////////////////////////////////////////
// ------------------------------- GETTERS --------------------------------- //
///////////////////////////////////////////////////////////////////////////////
XMFLOAT3 Transform::GetPosition() { return position; }
XMFLOAT3 Transform::GetRotation() { return rotation; }
XMFLOAT3 Transform::GetScale() { return scale; }

// --------------------------------------------------------
// If any changes have been made to the position, rotation,
// or scale, recalculates the world matrix as well as the
// world inverse transpose matrix.
// Returns the world matrix.
// --------------------------------------------------------
XMFLOAT4X4 Transform::GetWorldMatrix()
{
    // If no changes to position, rotation, or scale,
    // no need to recalculate, just return world matrix
    if (!dirtyWorld) {
        return worldMatrix;
    }

    // Otherwise, time to recalculate that world matrix
    // First, create matrices for translation, rotation, and scale
    XMMATRIX t = XMMatrixTranslation(
        position.x, position.y, position.z);
    XMMATRIX r = XMMatrixRotationRollPitchYaw(
        rotation.x, rotation.y, rotation.z);
    XMMATRIX s = XMMatrixScaling(
        scale.x, scale.y, scale.z);

    // Multiply S * R * T for the most predictable result
    // Using multiply method because supposedly it is faster
    XMMATRIX world = XMMatrixMultiply(XMMatrixMultiply(s, r), t);

    // Finally, store that result back in the matrix fields
    XMStoreFloat4x4(&worldMatrix, world);
    XMStoreFloat4x4(&worldInverseTransposeMatrix,
        XMMatrixInverse(0, XMMatrixTranspose(world)));

    // The world has been cleaned
    dirtyWorld = false;
    return worldMatrix;
}

// --------------------------------------------------------
// The world inverse transpose matrix is calculated along
// with the regular world matrix in GetWorldMatrix, so
// this method is just a simple getter.
// --------------------------------------------------------
XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix()
{
    return worldInverseTransposeMatrix;
}


///////////////////////////////////////////////////////////////////////////////
// ------------------------------- SETTERS --------------------------------- //
///////////////////////////////////////////////////////////////////////////////
void Transform::SetPosition(float x, float y, float z)
{
    dirtyWorld = true;
    position = XMFLOAT3(x, y, z);
}

void Transform::SetPosition(XMFLOAT3 _position)
{
    position = _position;
    dirtyWorld = true;
}

void Transform::SetRotation(float pitch, float yaw, float roll)
{
    rotation = XMFLOAT3(pitch, yaw, roll);
    dirtyWorld = true;
}

void Transform::SetRotation(XMFLOAT3 _rotation)
{
    rotation = _rotation;
    dirtyWorld = true;
}

void Transform::SetScale(float x, float y, float z)
{
    scale = XMFLOAT3(x, y, z);
    dirtyWorld = true;
}

void Transform::SetScale(XMFLOAT3 _scale)
{
    scale = _scale;
    dirtyWorld = true;
}


///////////////////////////////////////////////////////////////////////////////
// ------------------------------ MUTATORS --------------------------------- //
///////////////////////////////////////////////////////////////////////////////
void Transform::MoveAbsolute(float x, float y, float z)
{
    position.x += x;
    position.y += y;
    position.z += z;
    dirtyWorld = true;
}

void Transform::MoveAbsolute(XMFLOAT3 offset)
{
    position.x += offset.x;
    position.y += offset.y;
    position.z += offset.z;
    dirtyWorld = true;
}

// --------------------------------------------------------
// Moves relative to the transform's current rotation,
// which may not line up with the world axes.
// --------------------------------------------------------
void Transform::MoveRelative(float x, float y, float z)
{
    // Store the relative movement in an XMVECTOR
    // so we can do math with it
    XMVECTOR move = XMVectorSet(x, y, z, 0);

    // Store quaternion of the rotation using this
    // amazingly descriptive function name
    XMVECTOR rotQuat = XMQuaternionRotationRollPitchYawFromVector(
        XMLoadFloat3(&rotation));
    
    // Rotate movement by the rotation to get the direction
    // that the transform should actually move
    XMVECTOR dir = XMVector3Rotate(move, rotQuat);

    // Make an XMVECTOR of the position, add to it,
    // then store the result back in the position
    XMStoreFloat3(&position, XMLoadFloat3(&position) + dir);
}

void Transform::MoveRelative(XMFLOAT3 offset)
{
    // Don't repeat code, just call the overload
    MoveRelative(offset.x, offset.y, offset.z);
}

void Transform::Rotate(float pitch, float yaw, float roll)
{
    rotation.x += pitch;
    rotation.y += yaw;
    rotation.z += roll;
    dirtyWorld = true;
}

void Transform::Rotate(XMFLOAT3 _rotation)
{
    rotation.x += _rotation.x;
    rotation.y += _rotation.y;
    rotation.z += _rotation.z;
    dirtyWorld = true;
}

void Transform::Scale(float x, float y, float z)
{
    scale.x *= x;
    scale.y *= y;
    scale.z *= z;
    dirtyWorld = true;
}

void Transform::Scale(XMFLOAT3 _scale)
{
    scale.x *= _scale.x;
    scale.y *= _scale.y;
    scale.z *= _scale.z;
    dirtyWorld = true;
}

/*
William Duprey
9/25/24
Transform Class Header
*/

#pragma once
#include <DirectXMath.h>

// --------------------------------------------------------
// A class representing an object's position, 
// rotation, and scale within the world.
// --------------------------------------------------------
class Transform
{
public:
	Transform();
	~Transform();

	// Getters
	DirectX::XMFLOAT3 GetPosition();
	DirectX::XMFLOAT3 GetRotation();
	DirectX::XMFLOAT3 GetScale();
	DirectX::XMFLOAT4X4 GetWorldMatrix();
	DirectX::XMFLOAT4X4 GetWorldInverseTransposeMatrix();
	
	// Setters
	void SetPosition(float x, float y, float z);
	void SetPosition(DirectX::XMFLOAT3 position);
	void SetRotation(float pitch, float yaw, float roll);
	void SetRotation(DirectX::XMFLOAT3 rotation);
	void SetScale(float x, float y, float z);
	void SetScale(DirectX::XMFLOAT3 scale);

	// Mutators
	void MoveAbsolute(float x, float y, float z);
	void MoveAbsolute(DirectX::XMFLOAT3 offset);
	void Rotate(float pitch, float yaw, float roll);
	void Rotate(DirectX::XMFLOAT3 rotation);
	void Scale(float x, float y, float z);
	void Scale(DirectX::XMFLOAT3 scale);

private:
	// Whether the world matrix needs to be recalculated
	bool dirtyWorld;

	// Vectors used to construct the world matrix
	DirectX::XMFLOAT3 position;	
	DirectX::XMFLOAT3 rotation;	// Could be a float4 quaternion instead
	DirectX::XMFLOAT3 scale;	

	// Matrix fields, recalculated only when it is requested
	// and the above vectors have been updated
	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT4X4 worldInverseTranspose;
};


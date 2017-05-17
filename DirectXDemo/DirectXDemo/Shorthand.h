/*----------------------------------------------------------------
Author:			Orlando Cazalet-Hyams
Description :	A bunch of short-hand macros for a range of tasks.
				Not best practice, I don't encourage use of this ^_^
----------------------------------------------------------------*/
#pragma once

//DirectX Shorthand
//DX maths
#define LF2(_x) DirectX::XMLoadFloat2(_x)
#define LF3(_x) DirectX::XMLoadFloat3(_x)
#define LF4(_x) DirectX::XMLoadFloat4(_x)
#define LF44(_x) DirectX::XMLoadFloat4x4(_x)
#define VEC4(x, y, z, w) DirectX::XMVECTOR(LF4(&DirectX::XMFLOAT4(x, y , z, w)))
#define VEC3(x, y, z) DirectX::XMVECTOR(LF3(&DirectX::XMFLOAT3(x, y , z)))
#define VEC2(x, y)DirectX::XMVECTOR(LF2(&DirectX::XMFLOAT2(x, y)))
#define STOREF3(_dst, _src) DirectX::XMStoreFloat3(_dst, _src)
#define STOREF4(_dst, _src) DirectX::XMStoreFloat4(_dst, _src)
#define QUAT(_euler) DirectX::XMQuaternionRotationRollPitchYawFromVector(_euler)
#define TRANSPOSE(_matrix) DirectX::XMMatrixTranspose(_matrix)
//DX Resources
#define RELEASE(x) if (x) { x->Release(); x = nullptr; }

//C++ generic shorthand
//STL containers
#define ERASE_REMOVE(_container, _element) _container.erase(std::remove(_container.begin(), _container.end(), _element))
#define ERASE_REMOVE_IF(_container, _pred) _container.erase(std::remove_if(_container.begin(), _container.end(), _pred))
//Early out
#define RETURN_IF_FAILED(x) if (!x) return false
//sizeof() for buffers
#define SIZEOF_ROUND_TO_BOUNDARY(type, boundary) (sizeof(type) % boundary == 0? sizeof(type) : (std::ceil((double)sizeof(type) / (double)boundary) * boundary))
//nullify an array of points
#define NULLIFY_STATIC_ARRAY_OF_PTR(staticArray) for (int i=0; i < ARRAYSIZE(staticArray); ++i) { staticArray[i] = nullptr; }  
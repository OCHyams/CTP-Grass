#pragma once

//DirectX Shorthand
#define LF3(_x) XMLoadFloat3(_x)
#define LF4(_x) XMLoadFloat4(_x)
#define LF44(_x) XMLoadFloat4x4(_x)

#define VEC3(x, y, z) XMVECTOR(LF3(&XMFLOAT3(x, y , z)))

#define STOREF3(_dst, _src) XMStoreFloat3(_dst, _src)
#define STOREF4(_dst, _src) XMStoreFloat4(_dst, _src)

#define MUL(_vec, _matrix) XMMatrixMultiply(_vec, _matrix)
#define QUAT(_euler) XMQuaternionRotationRollPitchYawFromVector(_euler)

#define TRANSPOSE(_matrix) XMMatrixTranspose(_matrix)



//C++ generic shorthand
#define ERASE_REMOVE(_container, _element) _container.erase(std::remove(_container.begin(), _container.end(), _element))
#define ERASE_REMOVE_IF(_container, _pred) _container.erase(std::remove_if(_container.begin(), _container.end(), _pred))
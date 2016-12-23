#pragma once

#define LF3(_x) XMLoadFloat3(_x)
#define LF4(_x) XMLoadFloat4(_x)

#define VEC3(x, y, z) XMVECTOR(LF3(&XMFLOAT3(x, y , z)))

#define STOREF3(_dst, _src) XMStoreFloat3(_dst, _src)
#define STOREF4(_dst, _src) XMStoreFloat4(_dst, _src)

#define MUL(_vec, _matrix) XMMatrixMultiply(_vec, _matrix)
#define QUAT(_euler) XMQuaternionRotationRollPitchYawFromVector(_euler)


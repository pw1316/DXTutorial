#pragma once
#include "stdafx.h"

#include <D3DX10math.h>

class Light
{
public:
    Light() = default;
    Light(const Light &rhs) = default;
    Light(Light &&rhs) = default;
    ~Light() = default;

    D3DXVECTOR4 m_diffuse{};
    D3DXVECTOR3 m_dir{};
};
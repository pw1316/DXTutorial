#pragma once
#include "stdafx.h"

#include <D3DX10math.h>

class Camera
{
public:
    Camera() = default;
    Camera(const Camera &rhs) = default;
    Camera(Camera &&rhs) = default;
    ~Camera() = default;

    void SetPos(FLOAT xx, FLOAT yy, FLOAT zz);
    void SetRot(FLOAT xx, FLOAT yy, FLOAT zz);
    D3DXVECTOR3 GetPos() const;
    D3DXVECTOR3 GetRot() const;

    void Render();
    void GetViewMatrix(D3DXMATRIX &m);
private:
    BOOL m_dirty = true;
    FLOAT m_x = 0.0f, m_y = 0.0f, m_z = 0.0f;
    FLOAT m_rotx = 0.0f, m_roty = 0.0f, m_rotz = 0.0f;
    D3DXMATRIX m_matrix{};
};
#include "stdafx.h"
#include "Camera.h"

void Camera::SetPos(FLOAT xx, FLOAT yy, FLOAT zz)
{
    m_x = xx;
    m_y = yy;
    m_z = zz;
    m_dirty = true;
}

void Camera::SetRot(FLOAT xx, FLOAT yy, FLOAT zz)
{
    m_rotx = xx;
    m_roty = yy;
    m_rotz = zz;
    m_dirty = true;
}

D3DXVECTOR3 Camera::GetPos() const
{
    return D3DXVECTOR3(m_x, m_y, m_z);
}

D3DXVECTOR3 Camera::GetRot() const
{
    return D3DXVECTOR3(m_rotx, m_roty, m_rotz);
}

void Camera::Render()
{
    D3DXVECTOR3 pos(m_x, m_y, m_z), up(0.0f, 1.0f, 0.0f), lookAt(0.0f, 0.0f, 1.0f);
    D3DXMATRIX rotMatrix{};
    D3DXMatrixRotationYawPitchRoll(&rotMatrix, m_roty, m_rotx, m_rotz);
    D3DXVec3TransformCoord(&lookAt, &lookAt, &rotMatrix);
    D3DXVec3TransformCoord(&up, &up, &rotMatrix);
    lookAt = pos + lookAt;
    D3DXMatrixLookAtLH(&m_matrix, &pos, &lookAt, &up);
    m_dirty = false;
}

void Camera::GetViewMatrix(D3DXMATRIX & m)
{
    if (m_dirty)
    {
        Render();
    }
    m = m_matrix;
}

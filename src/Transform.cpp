#include <D3D11.h>
#include <D3DX11.h>
#include "Transform.h"

//==============================================================================
Transform::Transform () : m_rotation(0.0f) {

  m_position.x = m_position.y = 0.0f;
  m_scale.x    = m_scale.y    = 1.0f;
  m_velocity.x = m_velocity.y = 0.0f;

}

//==============================================================================
Transform::~Transform () {
}

//==============================================================================
void Transform::GetWorldMatrix (XMMATRIX * worldMatrixOut) const {

    XMMATRIX translation = XMMatrixTranslation(m_position.x, m_position.y, 0.0f);
    XMMATRIX rotationZ   = XMMatrixRotationZ(m_rotation);
    XMMATRIX scale       = XMMatrixScaling(m_scale.x, m_scale.y, 1.0f);

    //*worldMatrixOut = translation * rotationZ * scale;
    *worldMatrixOut = scale * rotationZ * translation;

}

//==============================================================================
void Transform::SetPosition (const XMFLOAT2& position) {
    m_position = position;
}

//==============================================================================
void Transform::SetRotation (float rotation) {
    m_rotation = rotation;
}

//==============================================================================
void Transform::SetScale (const XMFLOAT2& scale) {
    m_scale = scale;
}

//==============================================================================
void Transform::SetVelocity (const XMFLOAT2& velocity) {
    m_velocity = velocity;
}

//==============================================================================
void Transform::SetVelocity (float x, float y) {
    m_velocity.x = x;
    m_velocity.y = y;
}

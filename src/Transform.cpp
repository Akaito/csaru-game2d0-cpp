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
void Transform::GetWorldFromModelMtx (Mtx44 * worldMatrixOut) const {

    Mtx44 translation;
    Mtx44 rotationZ;
    Mtx44 scale;

    Mtx44::BuildTranslate(m_position.x, m_position.y, 0.0f, &translation);
    Mtx44::BuildRotateZ(m_rotation, &rotationZ);
    Mtx44::BuildScale(m_scale.x, m_scale.y, 1.0f, &scale);

    //*worldMatrixOut = translation * rotationZ * scale;
    *worldMatrixOut = scale * rotationZ * translation;

}

//==============================================================================
void Transform::SetPosition (const Vec3 & position) {
    m_position = position;
}

//==============================================================================
void Transform::SetRotation (float rotation) {
    m_rotation = rotation;
}

//==============================================================================
void Transform::SetScale (const Vec3 & scale) {
    m_scale = scale;
}

//==============================================================================
void Transform::SetVelocity (const Vec3 & velocity) {
    m_velocity = velocity;
}

//==============================================================================
void Transform::SetVelocity (float x, float y) {
    m_velocity.x = x;
    m_velocity.y = y;
}

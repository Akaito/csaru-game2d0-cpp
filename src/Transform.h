#pragma once

class Transform {

private: // Data

    XMFLOAT2 m_position;
    float    m_rotation;
    XMFLOAT2 m_scale;
    
    XMFLOAT2 m_velocity;

public:

    // Constructors

    Transform(void);
    virtual ~Transform(void);
    
    // Queries

    XMMATRIX         GetWorldMatrix () const;
    const XMFLOAT2 & GetPosition () const { return m_position; }
    float            GetRotation () const { return m_rotation; }
    const XMFLOAT2 & GetScale () const    { return m_scale; }
    const XMFLOAT2 & GetVelocity () const { return m_velocity; }

    // Commands

    void SetPosition (const XMFLOAT2& position);
    void SetRotation (float rotation);
    void SetScale (const XMFLOAT2& scale);
    
    void SetVelocity (const XMFLOAT2& velocity);
    void SetVelocity (float x, float y);

};

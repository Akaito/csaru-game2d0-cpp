#pragma once

class Transform {

private: // Data

    Vec3  m_position;
    float m_rotation;
    Vec3  m_scale;
    
    Vec3  m_velocity; // Hacks! (?)

public:
    // Construction
    Transform(void);
    virtual ~Transform(void);
    
    // Queries
    void         GetWorldFromModelMtx (Mtx44 * worldMatrixOut) const;
    const Vec3 & GetPosition () const     { return m_position; }
    float        GetRotation () const     { return m_rotation; }
    const Vec3 & GetScale () const        { return m_scale; }
    const Vec3 & GetVelocity () const     { return m_velocity; }

    // Commands
    void SetPosition (const Vec3 & position);
    void SetRotation (float rotation);
    void SetScale (const Vec3 & scale);
    
    void SetVelocity (const Vec3 & velocity);
    void SetVelocity (float x, float y);

};

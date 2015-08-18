#include "GameObject.h"
#include "GameObjectComponent.h"

//==============================================================================
GameObject::GameObject () {
    m_transform.SetPosition(Vec3(0.0f, 0.0f, 0.0f));
    m_transform.SetVelocity(Vec3(0.0f, 0.0f, 0.0f));
    m_transform.SetScale(Vec3(1.0f, 1.0f, 1.0f));
}

//==============================================================================
void GameObject::AddComponent (GameObjectComponent * component) {
    m_components.push_back(component);
    component->SetOwner(this);
}

//==============================================================================
GameObjectComponent * GameObject::GetComponent (unsigned componentType) {

    for (GameObjectComponent * goc : m_components) {
        if (goc->GetGlobalTypeId() == componentType)
            return goc;
    }

    return nullptr;

}

//==============================================================================
void GameObject::Render () {

    for (unsigned i = 0; i < m_components.size(); ++i) {
        GameObjectComponent * comp = m_components[i];
        comp->Render();
    }

}

//==============================================================================
void GameObject::Update (float dt) {

    for (unsigned i = 0; i < m_components.size(); ++i) {
        GameObjectComponent * comp = m_components[i];
        comp->Update(dt);
    }

}

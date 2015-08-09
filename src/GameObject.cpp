#include "GameObject.h"
#include "GameObjectComponent.h"

//==============================================================================
GameObject::GameObject ()
{}

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

#include "Engine/Renderer/transform.h"

Transform Transform::IDENTITY;

Transform::Transform()
    :m_local(Matrix4::IDENTITY)
    ,m_parent(nullptr)
{
}

Transform::Transform(Matrix4 local, const Transform* parent)
    :m_local(local)
    ,m_parent(parent)
{
}

void Transform::set_parent(const Transform* parent)
{
    if(nullptr == parent){
        m_parent = &Transform::IDENTITY;
    }else{
        m_parent = parent;
    }
}

Matrix4 Transform::calc_world_matrix() const
{
    if(nullptr == m_parent){
        return m_local;
    }

    return m_local * m_parent->calc_world_matrix();
}

Transform Transform::calc_world_transform() const
{
    return Transform(calc_world_matrix(), nullptr);
}

void Transform::look_at(const Vector3& location, const Vector3& up)
{
    Vector3 center = m_local.get_translation().xyz;

	Vector3 k_basis = (center - location).Normalized();
	Vector3 i_basis = CrossProduct(up, k_basis);
    Vector3 j_basis = CrossProduct(k_basis, i_basis);

	// Set basis vectors of transform matrix
	m_local.set_i_basis(i_basis, 0.0f);
	m_local.set_j_basis(j_basis, 0.0f);
	m_local.set_k_basis(k_basis, 0.0f);

}
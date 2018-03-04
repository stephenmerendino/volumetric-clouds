#pragma once

#include "Engine/Math/Matrix4.hpp"

class Transform
{
public:
    Matrix4           m_local;
    const Transform*  m_parent;

public:
    Transform();
    Transform(Matrix4 local, const Transform* parent = nullptr);

    void        set_parent(const Transform* parent);
    Matrix4     calc_world_matrix() const;
    Transform   calc_world_transform() const;
    void        look_at(const Vector3& location, const Vector3& up = Vector3::Y_AXIS);

    static Transform IDENTITY;
};
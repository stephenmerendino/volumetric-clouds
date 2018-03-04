#pragma once

class RenderableMesh;

#include <vector>

class Scene
{
    public:
        std::vector<RenderableMesh*> m_renderable_meshes;

    public:
        Scene();
        ~Scene();

    public:
        RenderableMesh* add_cube();
};
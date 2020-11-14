#pragma once
#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <memory>

#include <glm/glm.hpp>

#include "Terrain.h"

class MatrixStack;
class Program;

class Scene
{
public:
    Scene();
    Scene(float d, int n, bool flat = false);
    virtual ~Scene();
    void generateScene(bool flat = false);
    float getAltitude(float x, float z) { return terrain->getAltitude(x, z); }
    const std::shared_ptr<Terrain> getTerrain() { return terrain; }

    void draw(std::shared_ptr<MatrixStack> MV, const std::shared_ptr<Program> p_terrain);

private:
    std::shared_ptr<Terrain> terrain;

};

#endif
#pragma once
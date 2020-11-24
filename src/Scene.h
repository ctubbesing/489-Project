#pragma once
#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <memory>

#include <glm/glm.hpp>

#include "Terrain.h"

class Entity;
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
    const std::vector< std::shared_ptr<Entity> > getEntities() { return entities; }/////////////////////////////////prolly return reference tho
    float getSize() { return terrain->getSize(); }
    //void setProgTerrain(std::shared_ptr<Program> prog) { terrain->setProgram(prog); }
    void addEntity(std::shared_ptr<Entity> ent) { entities.push_back(ent); }

    void setProgSimple(std::shared_ptr<Program> prog) { progSimple = prog; }
    void setProgShapes(std::shared_ptr<Program> prog) { progShapes = prog; }
    void setProgSkin(std::shared_ptr<Program> prog) { progSkin = prog; }
    void setProgTerrain(std::shared_ptr<Program> prog)
    {
        progTerrain = prog;
        terrain->setProgram(prog);
    }

    void draw(std::shared_ptr<MatrixStack> P, std::shared_ptr<MatrixStack> MV);

private:
    std::shared_ptr<Terrain> terrain;
    std::vector< std::shared_ptr<Entity> > entities;

    std::shared_ptr<Program> progSimple;
    std::shared_ptr<Program> progShapes;
    std::shared_ptr<Program> progSkin;
    std::shared_ptr<Program> progTerrain;
};

#endif
#pragma once

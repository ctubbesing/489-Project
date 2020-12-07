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

class Scene : std::enable_shared_from_this<Scene>
{
public:
    Scene();
    Scene(float _edgeLength, int _edgeCells, bool flat = false, int unitsPerPGNode = 10, std::string _DATA_DIR = "");
    virtual ~Scene();
    void init();
    void generateScene(bool flat = false);
    float getAltitude(glm::vec3 pos) { return terrain->getAltitude(pos); }
    bool isObstacle(glm::vec3 pos) { return terrain->isObstacle(pos); }
    const std::shared_ptr<Terrain> getTerrain() { return terrain; }
    const std::vector< std::shared_ptr<Entity> > getEntities() { return entities; }
    float getEdgeLength() { return terrain->getEdgeLength(); }
    void addEntity(std::shared_ptr<Entity> ent) { entities.push_back(ent); }

    void setProgSimple(std::shared_ptr<Program> prog) { progSimple = prog; }
    void setProgShapes(std::shared_ptr<Program> prog) { progShapes = prog; }
    void setProgSkin(std::shared_ptr<Program> prog) { progSkin = prog; }
    void setProgTerrain(std::shared_ptr<Program> prog)
    {
        progTerrain = prog;
        terrain->setProgram(prog);
    }

    void draw(std::shared_ptr<MatrixStack> P, std::shared_ptr<MatrixStack> MV, double t, bool drawPG = true, bool drawPath = true);

private:
    std::shared_ptr<Terrain> terrain;
    std::vector< std::shared_ptr<Entity> > entities;

    std::shared_ptr<Program> progSimple;
    std::shared_ptr<Program> progShapes;
    std::shared_ptr<Program> progSkin;
    std::shared_ptr<Program> progTerrain;

    std::string DATA_DIR;
};

#endif
#pragma once

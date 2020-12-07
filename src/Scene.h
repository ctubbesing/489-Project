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

class ProgInfo
{
public:
    ProgInfo(std::shared_ptr<Program> _simple, std::shared_ptr<Program> _shapes, std::shared_ptr<Program> _skin) :
        progSimple(_simple),
        progShapes(_shapes),
        progSkin(_skin) {}
    std::shared_ptr<Program> progSimple;
    std::shared_ptr<Program> progShapes;
    std::shared_ptr<Program> progSkin;
};

class Scene : public std::enable_shared_from_this<Scene>
{
public:
    Scene();
    Scene(float _edgeLength, int _edgeCells, bool flat = false, std::string _DATA_DIR = "");
    virtual ~Scene();

    std::shared_ptr<Entity> init();
    void generateScene(bool flat = false);
    
    float getAltitude(glm::vec3 pos) { return terrain->getAltitude(pos); }
    bool isObstacle(glm::vec3 pos) { return terrain->isObstacle(pos); }
    
    const std::shared_ptr<Terrain> getTerrain() { return terrain; }
    const std::vector< std::shared_ptr<Entity> > getEntities() { return entities; }
    float getEdgeLength() { return terrain->getEdgeLength(); }
    
    std::shared_ptr<Entity> addEntity();
    //std::shared_ptr<Entity> addEntity(std::shared_ptr<Entity> ent) { entities.push_back(ent); }
    std::shared_ptr<Entity> deleteEntity();
    std::shared_ptr<Entity> selectEntity();

    void setProgSimple(std::shared_ptr<Program> prog) { progSimple = prog; }
    void setProgShapes(std::shared_ptr<Program> prog) { progShapes = prog; }
    void setProgSkin(std::shared_ptr<Program> prog) { progSkin = prog; }
    void setProgTerrain(std::shared_ptr<Program> prog);

    void draw(std::shared_ptr<MatrixStack> P, std::shared_ptr<MatrixStack> MV, double t, bool drawPG = true, bool drawPath = true);

private:
    std::shared_ptr<Terrain> terrain;
    std::vector< std::shared_ptr<Entity> > entities;
    
    int selectedEnt;

    std::shared_ptr<Program> progSimple;
    std::shared_ptr<Program> progShapes;
    std::shared_ptr<Program> progSkin;
    std::shared_ptr<Program> progTerrain;

    std::string DATA_DIR;
};

#endif
#pragma once

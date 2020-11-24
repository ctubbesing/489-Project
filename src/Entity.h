#pragma once
#ifndef ENTITY_H
#define ENTITY_H

#include <vector>
#include <memory>

#include <glm/glm.hpp>

#include "PathGraph.h"
//#include "ShapeSkin.h"
//#include "MatrixStack.h"

class MatrixStack;
class ShapeSkin;
class Program;
class Shape;

class Entity
{
public:
    Entity();
    Entity(glm::vec3 _pos, float sceneEdgeLength = 100.0f, int unitsPerPGNode = 10);
    virtual ~Entity();
    void setSkinProgram(std::shared_ptr<Program> _prog) { progSkin = _prog; }
    //void setPG(std::shared_ptr<PathGraph> _pg) { pg = _pg; }
    void setSkin(std::shared_ptr<Shape> _skin) { skin = _skin; }//////////////////////////////////temp
    //void setSkin(std::shared_ptr<ShapeSkin> _skin) { skin = _skin;
    void regenPG() { pg->regenerate(); }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    void setPGProgs(std::shared_ptr<Program> simpleProg, std::shared_ptr<Program> shapeProg)
    {
        pg->setSimpleProgram(simpleProg);
        pg->setShapeProgram(shapeProg);
    }
    void setPGShape(std::shared_ptr<Shape> shape) { pg->setShape(shape); }
    void setPos(glm::vec3 _pos);
    void setGoal(glm::vec3 _goal);
    ///////////////////////////////////////////////////////////////////////////////////////////////////


    void draw(std::shared_ptr<MatrixStack> P, std::shared_ptr<MatrixStack> MV);

private:
    glm::vec3 pos;
    float rot;
    glm::vec3 goal;

    void generatePath();

    std::shared_ptr<PathGraph> pg;
    std::shared_ptr<Shape> skin;//////////////////////////////////////////////////////////////////temp until ShapeSkin works better
    //std::shared_ptr<ShapeSkin> skin;
    std::vector<glm::vec3> path;
    std::shared_ptr<Program> progSkin;

};

#endif
#pragma once

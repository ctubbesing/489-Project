#pragma once
#ifndef ENTITY_H
#define ENTITY_H

#include <vector>
#include <memory>

#include <glm/glm.hpp>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
//#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include "PathGraph.h"
//#include "ShapeSkin.h"
//#include "MatrixStack.h"

class MatrixStack;
class ShapeSkin;
class Program;
class Shape;
class Texture;

class Entity
{
public:
    Entity();
    Entity(glm::vec3 _pos, float sceneEdgeLength = 100.0f, int unitsPerPGNode = 10);
    virtual ~Entity();
    void setSkinProgram(std::shared_ptr<Program> _prog) { progSkin = _prog; }
    //void setPG(std::shared_ptr<PathGraph> _pg) { pg = _pg; }
    //void setSkin(std::shared_ptr<Shape> _skin) { skin = _skin; }//////////////////////////////////temp
    void setSkin(std::vector< std::shared_ptr<ShapeSkin> > _skin) { skin = _skin; }
    void regenPG();

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    void setPGProgs(std::shared_ptr<Program> simpleProg, std::shared_ptr<Program> shapeProg)
    {
        pg->setSimpleProgram(simpleProg);
        pg->setShapeProgram(shapeProg);
    }
    void setPGShape(std::shared_ptr<Shape> shape) { pg->setShape(shape); }
    void setPos(glm::vec3 _pos);
    void setGoal(glm::vec3 _goal);

    void setTexMap(const std::map< std::string, std::shared_ptr<Texture> > &_textureMap) { textureMap = _textureMap; }
    void setBindPose(const std::vector<glm::mat4> &_bindPose) { bindPose = _bindPose; }
    void setFrames(const std::vector< std::vector<glm::mat4> > &_frames) { frames = _frames; }
    ///////////////////////////////////////////////////////////////////////////////////////////////////


    void draw(std::shared_ptr<MatrixStack> P, std::shared_ptr<MatrixStack> MV, double t);

private:
    glm::vec3 pos;
    float rot;
    glm::vec3 goal;

    void generatePath();

    std::shared_ptr<PathGraph> pg;
    //std::shared_ptr<Shape> skin;//////////////////////////////////////////////////////////////////temp until ShapeSkin works better
    std::vector< std::shared_ptr<ShapeSkin> > skin;

    /////////////////////////////////////////////////////////////////////////
    std::map< std::string, std::shared_ptr<Texture> > textureMap;
    std::vector<glm::mat4> bindPose;
    std::vector< std::vector<glm::mat4> > frames;
    /////////////////////////////////////////////////////////////////////////


    std::vector<glm::vec3> path;
    std::shared_ptr<Program> progSkin;

};

#endif
#pragma once

#pragma once
#ifndef ENTITY_H
#define ENTITY_H

#include <vector>
#include <memory>
#include <map>

#include <glm/glm.hpp>

#include "PathGraph.h"
//#include "ShapeSkin.h"
//#include "MatrixStack.h"

class MatrixStack;
class ShapeSkin;
class Program;
class Shape;
class Texture;

class SkinInfo {
public:
    SkinInfo(
        std::vector< std::shared_ptr<ShapeSkin> > &_skins,
        std::vector< std::vector< std::vector<glm::mat4> > > &_frames,
        std::vector<glm::mat4> &_bindPose,
        std::map< std::string, std::shared_ptr<Texture> > &_textureMap
    ) : skins(_skins), frames(_frames), bindPose(_bindPose), textureMap(_textureMap) {}
//
    std::vector< std::shared_ptr<ShapeSkin> > skins;
    std::vector< std::vector< std::vector<glm::mat4> > > frames;
    std::vector<glm::mat4> bindPose;
    std::map< std::string, std::shared_ptr<Texture> > textureMap;
};

class DataInput
{
public:
    vector<string> textureData;
    vector< vector<string> > meshData;
    vector<string> skeletonData;
};

class Entity
{
public:
    Entity();
    Entity(glm::vec3 _pos, const std::shared_ptr<Scene> _scene, float sceneEdgeLength = 100.0f, int unitsPerPGNode = 10);
    virtual ~Entity();
    void setSkinProgram(std::shared_ptr<Program> _prog) { progSkin = _prog; }
    //void setPG(std::shared_ptr<PathGraph> _pg) { pg = _pg; }
    //void setSkin(std::shared_ptr<Shape> _skin) { skin = _skin; }//////////////////////////////////temp
    void setSkin(std::vector< std::shared_ptr<ShapeSkin> > &_skin) { skins = _skin; }
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
    void setSkinInfo(SkinInfo &s);
    void speedUp() { speed += 3.0f; }
    void resetSpeed() { speed = 7.0f; }
    ///////////////////////////////////////////////////////////////////////////////////////////////////

    void update(double _t);
    void draw(std::shared_ptr<MatrixStack> P, std::shared_ptr<MatrixStack> MV, bool drawPG = true, bool drawPath = true);


protected:
    glm::vec3 pos;
    glm::mat4 rot;
    glm::vec3 goal;

    float speed;

    enum EntityState {
        IDLE,
        TRAVELING
    } state;

    double t, t0;

    void loadSkeletonData(std::string entityType);
    void generatePath();

    std::shared_ptr<PathGraph> pg;
    std::vector<glm::vec3> path;
    std::vector< std::pair<float, float> > usTable;
    //std::shared_ptr<Shape> skin;//////////////////////////////////////////////////////////////////temp until ShapeSkin works better

    std::vector< std::shared_ptr<ShapeSkin> > skins;
    std::vector< std::vector< std::vector<glm::mat4> > > frames;
    std::vector<glm::mat4> bindPose;
    std::map< std::string, std::shared_ptr<Texture> > textureMap;
    int currentFrame;

    std::shared_ptr<Program> progSkin;

    const std::shared_ptr<Scene> scene;
};

#endif
#pragma once

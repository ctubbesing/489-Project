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
class ProgInfo;

//class SkinInfo {
//public:
//    SkinInfo(
//        std::vector< std::shared_ptr<ShapeSkin> > &_skins,
//        std::vector< std::vector< std::vector<glm::mat4> > > &_frames,
//        std::vector<glm::mat4> &_bindPose,
//        std::map< std::string, std::shared_ptr<Texture> > &_textureMap
//    ) : skins(_skins), frames(_frames), bindPose(_bindPose), textureMap(_textureMap) {}
////
//    std::vector< std::shared_ptr<ShapeSkin> > skins;
//    std::vector< std::vector< std::vector<glm::mat4> > > frames;
//    std::vector<glm::mat4> bindPose;
//    std::map< std::string, std::shared_ptr<Texture> > textureMap;
//};

struct DataInput
{
    std::string entityType;
    std::string DATA_DIR;
    std::vector<std::string> textureData;
    std::vector< std::vector<std::string> > meshData;
    std::vector<std::string> skeletonData;
};

class Entity
{
public:
    Entity();
    Entity(
        std::string ENTITY_TYPE,
        glm::vec3 _pos,
        const std::shared_ptr<Scene> _scene,
        ProgInfo progs,
        //std::shared_ptr<Program> _progSkin,
        std::string DATA_DIR = ""
    );
    virtual ~Entity();
    //void setProgs(ProgInfo progs);
    void regenPG();

    void setPos(glm::vec3 _pos);
    void setGoal(glm::vec3 _goal);

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

    void loadDataInputFile(DataInput &dataInput);
    void loadSkeletonData(const DataInput &dataInput);
    //void init(const DataInput &dataInput);
    void generatePath();

    std::shared_ptr<PathGraph> pg;
    std::vector<glm::vec3> path;
    std::vector< std::pair<float, float> > usTable;

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

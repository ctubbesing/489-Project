#pragma once
#ifndef BIGVEGAS_H
#define BIGVEGAS_H

#include <vector>
#include <memory>
//#include <map>

#include <glm/glm.hpp>

#include "Entity.h"
//#include "PathGraph.h"
//#include "ShapeSkin.h"
//#include "MatrixStack.h"

//class MatrixStack;
//class ShapeSkin;
class Program;
//class Shape;
//class Texture;

//struct SkinInfo {
//    SkinInfo(
//        std::vector< std::shared_ptr<ShapeSkin> > &_skins,
//        std::vector< std::vector< std::vector<glm::mat4> > > &_frames,
//        std::vector<glm::mat4> &_bindPose,
//        std::map< std::string, std::shared_ptr<Texture> > &_textureMap
//    ) : skins(_skins), frames(_frames), bindPose(_bindPose), textureMap(_textureMap) {}
//    //
//    std::vector< std::shared_ptr<ShapeSkin> > skins;
//    std::vector< std::vector< std::vector<glm::mat4> > > frames;
//    std::vector<glm::mat4> bindPose;
//    std::map< std::string, std::shared_ptr<Texture> > textureMap;
//};

class BigVegas : public Entity
{
public:
    BigVegas();
    BigVegas(
        std::shared_ptr<Program> _progSkin,
        glm::vec3 _pos,
        const std::shared_ptr<Scene> _scene,
        float sceneEdgeLength = 100.0f,
        int unitsPerPGNode = 10,
        std::string DATA_DIR = ""
    );
    virtual ~BigVegas();
};

#endif
#pragma once

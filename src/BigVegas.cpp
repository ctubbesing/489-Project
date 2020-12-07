#include "BigVegas.h"
//#include "Terrain.h"
//#include "Entity.h"
//#include "Program.h"

using namespace std;

BigVegas::BigVegas() :
    Entity()
{

}

BigVegas::BigVegas(glm::vec3 _pos, const std::shared_ptr<Scene> _scene, float sceneEdgeLength, int unitsPerPGNode, string DATA_DIR) :
    Entity(_pos, _scene, sceneEdgeLength, unitsPerPGNode)
{
    // Create skin shapes
    for (const auto &mesh : dataInput.meshData) {
        auto shape = make_shared<ShapeSkin>();
        shapes.push_back(shape);
        shape->setTextureMatrixType(mesh[0]);
        shape->loadMesh(DATA_DIR + mesh[0]);
        shape->loadAttachment(DATA_DIR + mesh[1]);
        shape->setTextureFilename(mesh[2]);
    }

    loadSkeletonData();

    for (auto shape : shapes) {
        shape->init();
    }

}

BigVegas::~BigVegas()
{

}

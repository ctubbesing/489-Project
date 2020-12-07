#include "BigVegas.h"
#include "ShapeSkin.h"
#include "Program.h"
#include "Texture.h"
//#include "Entity.h"

using namespace std;

BigVegas::BigVegas() :
    Entity()
{

}

BigVegas::BigVegas(
    shared_ptr<Program> _progSkin,
    glm::vec3 _pos,
    const std::shared_ptr<Scene> _scene,
    float sceneEdgeLength,
    int unitsPerPGNode,
    string DATA_DIR
) :
    Entity(_pos, _scene, _progSkin, sceneEdgeLength, unitsPerPGNode)
{
    // load data from input file
    DataInput dataInput;
    dataInput.DATA_DIR = DATA_DIR;
    loadDataInputFile(dataInput);
    //DataInput dataInput = loadDataInputFile(DATA_DIR);

    // Create skin shapes
    for (const auto &mesh : dataInput.meshData) {
        shared_ptr<ShapeSkin> shape = make_shared<ShapeSkin>();
        skins.push_back(shape);
        shape->setTextureMatrixType(mesh[0]);
        shape->loadMesh(DATA_DIR + mesh[0]);
        shape->loadAttachment(DATA_DIR + mesh[1]);
        shape->setTextureFilename(mesh[2]);
    }

    loadSkeletonData(dataInput, DATA_DIR);

    init(dataInput);
}

BigVegas::~BigVegas()
{

}

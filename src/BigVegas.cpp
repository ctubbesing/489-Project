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
    ///////////// !!! in input-file-method branch, this input file is what is used to customize which skin, skeleton,
    /////////////     animation, etc files are read into the given Entity. The method done in the other branch is to
    /////////////     create a child of Entity (e.g. BigVegas, Monster, Soldier) and hard-code the filenames into its
    /////////////     constructor. tbh idk which is the better way to do it
    DataInput dataInput;
    dataInput.entityType = "BigVegas";
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

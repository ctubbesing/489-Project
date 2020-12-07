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
    glm::vec3 _pos,
    const std::shared_ptr<Scene> _scene,
    shared_ptr<Program> _progSkin,
    float sceneEdgeLength,
    int unitsPerPGNode,
    string DATA_DIR
) :
    Entity(string("BigVegas"), _pos, _scene, _progSkin, unitsPerPGNode, DATA_DIR)
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

    init(dataInput);
}

BigVegas::~BigVegas()
{

}

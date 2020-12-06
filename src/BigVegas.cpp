#include "BigVegas.h"
//#include "Terrain.h"
//#include "Entity.h"
//#include "Program.h"

using namespace std;

BigVegas::BigVegas() :
    Entity()
{

}

BigVegas::BigVegas(glm::vec3 _pos, const std::shared_ptr<Scene> _scene, float sceneEdgeLength, int unitsPerPGNode) :
    Entity(_pos, _scene, sceneEdgeLength, unitsPerPGNode)
{
    
}

BigVegas::~BigVegas()
{

}

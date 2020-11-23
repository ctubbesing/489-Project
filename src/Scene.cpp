#include "Scene.h"
#include "Terrain.h"

using namespace std;

Scene::Scene()
{
    terrain = make_shared<Terrain>();
}

Scene::Scene(float d, int n, bool flat)
{
    terrain = make_shared<Terrain>(d, n, flat);
}

Scene::~Scene()
{

}

void Scene::generateScene(bool flat)
{
    terrain->generateTerrain(flat);
}

void Scene::draw(shared_ptr<MatrixStack> MV)
{
    terrain->draw(MV);
}

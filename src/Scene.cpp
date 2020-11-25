#include "Scene.h"
#include "Terrain.h"
#include "Entity.h"
#include "Program.h"

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

void Scene::draw(std::shared_ptr<MatrixStack> P, std::shared_ptr<MatrixStack> MV, double t)
{
    progTerrain->bind();
    terrain->draw(P, MV);
    progTerrain->unbind();

    for (auto entity : entities) {
        entity->draw(P, MV, t);
    }
}

#include "Scene.h"
#include "Terrain.h"
#include "Entity.h"
#include "BigVegas.h"
#include "Program.h"

using namespace std;

Scene::Scene()
{
    terrain = make_shared<Terrain>();
}

Scene::Scene(float edgeLength, int _edgeCells, bool flat, int unitsPerPGNode, string _DATA_DIR) :
    DATA_DIR(_DATA_DIR)
{
    terrain = make_shared<Terrain>(edgeLength, _edgeCells, flat);

    entities.push_back(make_shared<BigVegas>(glm::vec3(0.0f), shared_from_this(), edgeLength, unitsPerPGNode, DATA_DIR));
}

Scene::~Scene()
{

}

void Scene::generateScene(bool flat)
{
    terrain->generateTerrain(flat);
}

void Scene::draw(std::shared_ptr<MatrixStack> P, std::shared_ptr<MatrixStack> MV, double t, bool drawPG, bool drawPath)
{
    progTerrain->bind();
    terrain->draw(P, MV);
    progTerrain->unbind();

    for (auto entity : entities) {
        entity->update(t);
        entity->draw(P, MV, drawPG, drawPath);
    }
}

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

Scene::Scene(float edgeLength, int _edgeCells, bool flat, string _DATA_DIR) :
    DATA_DIR(_DATA_DIR),
    selectedEnt(0)
{
    // create scene terrrain
    terrain = make_shared<Terrain>(edgeLength, _edgeCells, flat);
}

Scene::~Scene()
{

}

shared_ptr<Entity> Scene::init()
{
    shared_ptr<Scene> thisScene = shared_from_this();
    ProgInfo progs(progSimple, progShapes, progSkin);

    // create single BigVegas instance for scene
    shared_ptr<Entity> initialVegas = make_shared<Entity>(string("BigVegas"), glm::vec3(0.0f), thisScene, progs, DATA_DIR);
    entities.push_back(initialVegas);

    return initialVegas;
}

void Scene::generateScene(bool flat)
{
    terrain->generateTerrain(flat);
}

shared_ptr<Entity> Scene::selectEntity()
{
    // increment selected entity index and return pointer to entity
    selectedEnt = (selectedEnt + 1) % entities.size();

    return entities[selectedEnt];
}

shared_ptr<Entity> Scene::addEntity()
{
    // create a copy of the currently selected entity and add it to the scene
    shared_ptr<Entity> ent = entities[selectedEnt];

    shared_ptr newEnt = make_shared<Entity>(*ent);
    newEnt->setPos(glm::vec3(0.0f));
    newEnt->setGoal(glm::vec3(0.0f));

    entities.push_back(newEnt);
    selectedEnt = entities.size() - 1;

    return entities[selectedEnt];
}

shared_ptr<Entity> Scene::deleteEntity()
{
    if (entities.size() > 1) {
        entities.erase(entities.begin() + selectedEnt);

        selectedEnt %= entities.size();
    }
    return entities[selectedEnt];
}

void Scene::setProgTerrain(shared_ptr<Program> prog)
{
    progTerrain = prog;
    terrain->setProgram(prog);
}

void Scene::draw(std::shared_ptr<MatrixStack> P, std::shared_ptr<MatrixStack> MV, double t, bool drawPG, bool drawPath)
{
    progTerrain->bind();
    terrain->draw(P, MV);
    progTerrain->unbind();

    for (auto entity : entities) {
        entity->update(t);
        if (entity == entities[selectedEnt]) {
            entity->draw(P, MV, true, drawPG, drawPath);
        }
        else {
            entity->draw(P, MV);
        }
    }
}

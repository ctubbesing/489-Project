#include "Entity.h"
#include "Shape.h"
#include "ShapeSkin.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Texture.h"

#include "GLSL.h"
//#include "Camera.h"
//#include "Scene.h"
//#include "Terrain.h"
//#include "PathGraph.h"


#include <glm/gtc/type_ptr.hpp>

using namespace std;

class Program;

Entity::Entity() :
    pos(glm::vec3(0.0f, 0.0f, 0.0f)),
    goal(glm::vec3(0.0f, 0.0f, 0.0f)),
    rot(0.0f)
{
    pg = make_shared<PathGraph>();
}

Entity::Entity(glm::vec3 _pos, float sceneEdgeLength, int unitsPerPGNode) :
    pos(_pos),
    goal(glm::vec3(0.0f, 0.0f, 0.0f)),
    rot(0.0f)
{
    pg = make_shared<PathGraph>(sceneEdgeLength, unitsPerPGNode);
}

Entity::~Entity()
{

}

void Entity::generatePath()
{
    pg->updateStart(pos);
    pg->updateGoal(goal);

    path = pg->findPath();
}

void Entity::regenPG()
{
    pg->regenerate(pos, goal);
    generatePath();
}

void Entity::setPos(glm::vec3 _pos)
{
    pos = _pos;
    generatePath();
}

void Entity::setGoal(glm::vec3 _goal)
{
    goal = _goal;
    generatePath();
}

//void Entity::regenPG(int unitsPerNode)
//{
//    pg
//}

void Entity::draw(shared_ptr<MatrixStack> P, shared_ptr<MatrixStack> MV, double t)
{
    // draw skin
    //progSkin->bind();

    //glUniform3f(progSkin->getUniform("kd"), 0.2f, 0.5f, 0.6f);
    //glUniform3f(progSkin->getUniform("ka"), 0.02f, 0.05f, 0.06f);
    //glUniformMatrix4fv(progSkin->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));

    //MV->pushMatrix();

    //MV->translate(pos);
    //MV->rotate(rot, glm::vec3(0.0f, 1.0f, 0.0f));
    //MV->translate(glm::vec3(0.0f, 1.5f, 0.0f)); // shape offset prolly 0 anyways
    //glUniformMatrix4fv(progSkin->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
    //skin->draw();

    //MV->popMatrix();

    double fps = 30;
    int frameCount = frames.size();
    int frame = ((int)floor(t*fps)) % frameCount;

    for (const auto &shape : skin) {
        MV->pushMatrix();

        progSkin->bind();
        textureMap[shape->getTextureFilename()]->bind(progSkin->getUniform("kdTex"));
        glLineWidth(1.0f); // for wireframe
        MV->scale(0.05f);
        glUniformMatrix4fv(progSkin->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
        glUniformMatrix4fv(progSkin->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
        glUniform3f(progSkin->getUniform("ka"), 0.1f, 0.1f, 0.1f);
        glUniform3f(progSkin->getUniform("ks"), 0.1f, 0.1f, 0.1f);
        glUniform1f(progSkin->getUniform("s"), 200.0f);
        shape->setProgram(progSkin);
        shape->update(bindPose, frames[frame]);
        shape->draw();
        progSkin->unbind();

        MV->popMatrix();
    }


    //skin->draw();
    
    progSkin->unbind();

    pg->draw(P, MV, path);
}
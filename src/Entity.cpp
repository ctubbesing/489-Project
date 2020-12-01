#include <iostream>

#include "Entity.h"
#include "Shape.h"
#include "ShapeSkin.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Texture.h"

#include <glm/gtc/type_ptr.hpp>
#include <algorithm>

using namespace std;

class Program;

Entity::Entity() :
    pos(glm::vec3(0.0f)),
    goal(glm::vec3(0.0f)),
    rot(0.0f),
    state(IDLE),
    currentFrame(0)
{
    pg = make_shared<PathGraph>();
}

Entity::Entity(glm::vec3 _pos, float sceneEdgeLength, int unitsPerPGNode) :
    pos(_pos),
    goal(glm::vec3(0.0f)),
    rot(0.0f),
    state(IDLE),
    currentFrame(0)
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

void Entity::setSkinInfo(SkinInfo &s)
{
    skins = s.skins;
    frames = s.frames;
    bindPose = s.bindPose;
    textureMap = s.textureMap;
}

void Entity::update(double t)
{
    //switch (state) {
    //    case IDLE:
    //        if pos
    //        break;
    //    case TRAVELING:

    //        break;
    //}

    if (path.size() == 0) {
        return;
    }

    // define Catmull-Rom B matrix
    glm::mat4 B;
    B[0] = glm::vec4(0.0f, 2.0f, 0.0f, 0.0f);
    B[1] = glm::vec4(-1.0f, 0.0f, 1.0f, 0.0f);
    B[2] = glm::vec4(2.0f, -5.0f, 4.0f, -1.0f);
    B[3] = glm::vec4(-1.0f, 3.0f, -3.0f, 1.0f);
    B /= 2;

    int pathSegments = path.size();
    float u = (float)fmod(t, pathSegments);

    /* -- draw moving entity -- */
    int u_int = (int)floor(u);
    float u_frac = u - u_int;

    glm::vec4 uVec(1, u_frac, u_frac*u_frac, u_frac*u_frac*u_frac);

    // calculate G matrix for position
    glm::mat4 G_position;
    G_position[0] = glm::vec4(path[max(u_int - 1, 0)], 0.0f);
    G_position[1] = glm::vec4(path[u_int], 0.0f);
    G_position[2] = glm::vec4(path[min(u_int + 1, pathSegments - 1)], 0.0f);
    G_position[3] = glm::vec4(path[min(u_int + 2, pathSegments - 1)], 0.0f);
    //G_position[0] = glm::vec4(path[u_int], 0.0f);
    //G_position[1] = glm::vec4(path[(u_int + 1) % pathSegments], 0.0f);
    //G_position[2] = glm::vec4(path[(u_int + 2) % pathSegments], 0.0f);
    //G_position[3] = glm::vec4(path[(u_int + 3) % pathSegments], 0.0f);

    // update position
    glm::vec3 oldPos = pos;
    pos = G_position * (B*uVec);

    // update rotation
    if (pos != oldPos) {

        float angle_A = std::atan2(1.0f, 0.0f);
        cout << "atan2(1,0) = " << angle_A << endl;
        float angle_B = std::atan2(pos.z, pos.x);
        //float angle_B = std::atan2(pos.z - oldPos.z, pos.x - oldPos.x);

        float angle_from_A_to_B = angle_B - angle_A;
        float angle_from_B_to_A = angle_A - angle_B;

        rot = angle_B;
        //rot = atan(glm::dot(glm::vec3(1.0f, 0.0f, 0.0f), glm::normalize(pos - oldPos)));
        //if (pos.x > oldPos.x) {
        //    rot *= -1;
        //}
        cout << "rotation = " << rot << endl;
    }
    //rot = glm::lookAt(pos, oldPos, glm::vec3(0.0f, 1.0f, 0.0f));

    // update current frame
    double fps = 30;
    int frameCount = frames[state].size();
    currentFrame = ((int)floor(t*fps)) % frameCount;
}

void Entity::draw(shared_ptr<MatrixStack> P, shared_ptr<MatrixStack> MV)
{
    // draw skin
    MV->pushMatrix();
    MV->translate(pos);
    MV->rotate(rot, glm::vec3(0.0f, 1.0f, 0.0f));
    //MV->multMatrix(rot);

    for (const auto &shape : skins) {
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
        shape->update(bindPose, frames[state][currentFrame]);
        shape->draw();
        progSkin->unbind();

        MV->popMatrix();
    }

    MV->popMatrix();






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

    //skin->draw();
    
    //progSkin->unbind();

    pg->draw(P, MV, path);
}
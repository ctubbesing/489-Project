#include <iostream>

#include "Entity.h"
#include "Shape.h"
#include "ShapeSkin.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Texture.h"
#include "Scene.h"

#include <glm/gtc/type_ptr.hpp>
#include <algorithm>

using namespace std;

class Program;

Entity::Entity() :
    pos(glm::vec3(0.0f)),
    goal(glm::vec3(0.0f)),
    rot(glm::identity<glm::mat4>()),
    //rot(0.0f),
    state(IDLE),
    currentFrame(0),
    speed(7.0f),
    t(0),
    t0(0)
{
    pg = make_shared<PathGraph>();
}

Entity::Entity(glm::vec3 _pos, const shared_ptr<Scene> _scene, float sceneEdgeLength, int unitsPerPGNode) :
    pos(_pos),
    scene(_scene),
    goal(glm::vec3(0.0f)),
    rot(glm::identity<glm::mat4>()),
    //rot(0.0f),
    state(IDLE),
    currentFrame(0),
    speed(7.0f),
    t(0),
    t0(0)
{
    pg = make_shared<PathGraph>(scene, sceneEdgeLength, unitsPerPGNode);
}

Entity::~Entity()
{

}

void Entity::generatePath()
{
    pg->updateStart(pos);
    pg->updateGoal(goal);

    path = pg->findPath();
    usTable.clear();
    state = TRAVELING;
    t = 0;
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

float arcLength(float u_a, float u_b, glm::mat4 &B, glm::mat4 &G) {
    glm::vec4 uVec_a(1.0f, u_a, u_a*u_a, u_a*u_a*u_a);
    glm::vec4 uVec_b(1.0f, u_b, u_b*u_b, u_b*u_b*u_b);

    glm::vec4 P_ua = G * (B*uVec_a);
    glm::vec4 P_ub = G * (B*uVec_b);

    return glm::length(P_ub - P_ua);
}

void Entity::update(double _t)
{
    // update time
    double t1 = _t;
    double dt = (t1 - t0);
    t += dt;
    t0 = t1;

    switch (state) {
        case IDLE:
            //cout << "Idle." << endl;
            break;
        case TRAVELING:
            //cout << "Traveling." << endl;
            // switch to idle once goal is reached
            glm::vec3 distToGo(pos - goal);
            distToGo.y = 0.0f;
            float minDist = 1.0f;
            if (glm::length(distToGo) < minDist) {
                //cout << "close to goal; switching to idle" << endl;
                setPos(goal);
                state = IDLE;
                break;
            }

            if (path.size() == 0) {
                break;
            }

            /* -- calculate location along path -- */
            // define Catmull-Rom B matrix
            glm::mat4 B;
            B[0] = glm::vec4(0.0f, 2.0f, 0.0f, 0.0f);
            B[1] = glm::vec4(-1.0f, 0.0f, 1.0f, 0.0f);
            B[2] = glm::vec4(2.0f, -5.0f, 4.0f, -1.0f);
            B[3] = glm::vec4(-1.0f, 3.0f, -3.0f, 1.0f);
            B /= 2;

            // use arc-length parametrization to get u

            // generate u/s table if necessary
            int pathSegments = path.size();
            if (usTable.size() == 0) {
                float d_u = 0.05f;
                float u_table = 0.0f;
                float s_table = 0.0f;
                usTable.push_back(make_pair(u_table, s_table));

                // iterate for each curve
                for (int i = 0; i < pathSegments; i++) {
                    // calculate G for this curve
                    glm::mat4 G;
                    //G[0] = glm::vec4(keyframes[i], 0.0f);
                    //G[1] = glm::vec4(keyframes[(i + 1) % numCurves], 0.0f);
                    //G[2] = glm::vec4(keyframes[(i + 2) % numCurves], 0.0f);
                    //G[3] = glm::vec4(keyframes[(i + 3) % numCurves], 0.0f);

                    G[0] = glm::vec4(path[max(i - 1, 0)], 0.0f);
                    G[1] = glm::vec4(path[i], 0.0f);
                    G[2] = glm::vec4(path[min(i + 1, pathSegments - 1)], 0.0f);
                    G[3] = glm::vec4(path[min(i + 2, pathSegments - 1)], 0.0f);


                    u_table = 0.0f;
                    for (int j = 0; j < (1.0f / d_u); j++) {
                        s_table += arcLength(u_table, u_table + d_u, B, G);
                        u_table += d_u;

                        usTable.push_back(make_pair(u_table + i, s_table));
                    }
                }
            }

            // convert t to s and implement time control
            // update tMax
            //float v = 7; // units/sec
            float dist = usTable.back().second; // units
            float tMax = dist / speed;
            float tNorm = (float)fmod(t, tMax) / tMax;

            float sNorm = tNorm;
            //float sNorm;
            //switch (keyPresses[(unsigned)'s'] % 3) {
            //case 0: // default - normal time
            //    sNorm = tNorm;
            //    break;
            //case 1: // provided example - s = -2t^3 + 3t^2
            //    sNorm = -2 * (tNorm*tNorm*tNorm) + 3 * (tNorm*tNorm);
            //    break;
            //case 2: // custom example - sin function
            //    sNorm = (sin(t) + 1.5f) / 1.5f;
            //    break;
            //}

            float sMax = usTable.back().second;
            float s = sMax * sNorm;

            // convert s to u
            size_t n = 0;
            while (usTable[n].second <= s && n < (usTable.size() - 1)) {
                n++;
            }

            float s1 = usTable[n].second;
            float s0 = (n == 0 ? 0.0f : usTable[n - 1].second);
            float alpha;
            if (s1 - s0 == 0) {
                alpha = 0;
            }
            else {
                alpha = (s - s0) / (s1 - s0);
            }

            float u1 = usTable[n].first;
            float u0 = (n == 0 ? 0.0f : usTable[n - 1].first);
            float u = u0 + alpha * (u1 - u0);

            //float u = (float)fmod(t, pathSegments);

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
                glm::vec3 pos_flat(pos.x, 0.0f, pos.z);
                glm::vec3 oldPos_flat(oldPos.x, 0.0f, oldPos.z);
                rot = glm::inverse(glm::lookAt(pos_flat, oldPos_flat, glm::vec3(0.0f, 1.0f, 0.0f)));
            }

            break;
    }

    // update current frame
    double fps = 30;
    int frameCount = frames[state].size();
    currentFrame = ((int)floor(t*fps)) % frameCount;
}

void Entity::draw(shared_ptr<MatrixStack> P, shared_ptr<MatrixStack> MV, bool drawPG, bool drawPath)
{
    // draw skin
    MV->pushMatrix();
    //MV->rotate(rot, glm::vec3(0.0f, 1.0f, 0.0f));
    pos.y = scene->getAltitude(pos);
    MV->translate(pos);
    MV->scale(0.05f);
    MV->multMatrix(rot);

    for (const auto &shape : skins) {
        MV->pushMatrix();

        progSkin->bind();
        textureMap[shape->getTextureFilename()]->bind(progSkin->getUniform("kdTex"));
        glLineWidth(1.0f); // for wireframe
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
    pg->draw(P, MV, path, drawPG, drawPath);
}
#include <iostream>
#include <fstream>
#include <sstream>

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
    t(0),
    t0(0)
{
    pg = make_shared<PathGraph>();
}

Entity::Entity(
    string ENTITY_TYPE,
    glm::vec3 _pos,
    const shared_ptr<Scene> _scene,
    ProgInfo progs,
    //shared_ptr<Program> _progSkin,
    string DATA_DIR
) :
    pos(_pos),
    scene(_scene),
    progSkin(progs.progSkin),
    goal(glm::vec3(0.0f)),
    rot(glm::identity<glm::mat4>()),
    state(IDLE),
    currentFrame(0),
    t(0),
    t0(0)
{
    // create PathGraph
    pg = make_shared<PathGraph>(scene, progs, DATA_DIR);

    // load data from input.txt
    DataInput dataInput;
    dataInput.entityType = ENTITY_TYPE;
    dataInput.DATA_DIR = DATA_DIR;
    loadDataInputFile(dataInput);

    //init(dataInput);

    // Create skin shapes
    for (const auto &mesh : dataInput.meshData) {
        shared_ptr<ShapeSkin> shape = make_shared<ShapeSkin>();
        skins.push_back(shape);
        shape->setTextureMatrixType(mesh[0]);
        shape->loadMesh(dataInput.DATA_DIR + mesh[0]);
        shape->loadAttachment(dataInput.DATA_DIR + mesh[1]);
        shape->setTextureFilename(mesh[2]);
    }

    loadSkeletonData(dataInput);

    for (auto skin : skins) {
        skin->init();
    }

    // Bind the texture to unit 1.
    int unit = 1;
    progSkin->bind();
    glUniform1i(progSkin->getUniform("kdTex"), unit);
    progSkin->unbind();

    for (const auto &filename : dataInput.textureData) {
        auto textureKd = make_shared<Texture>();
        textureMap[filename] = textureKd;
        textureKd->setFilename(dataInput.DATA_DIR + filename);
        textureKd->init();
        textureKd->setUnit(unit); // Bind to unit 1
        textureKd->setWrapModes(GL_REPEAT, GL_REPEAT);
    }

    //pg->setSimpleProgram(simpleProg);
    //pg->setShapeProgram(shapeProg);
    //ent->setPGShape(pmShape);
}

Entity::Entity(const Entity &ent) :
    pos(ent.pos),
    rot(ent.rot),
    goal(ent.pos),
    speed(ent.speed),
    state(IDLE),
    pg(make_shared<PathGraph>(*ent.pg)),
    skins(ent.skins),
    frames(ent.frames),
    bindPose(ent.bindPose),
    textureMap(ent.textureMap),
    progSkin(ent.progSkin),
    scene(ent.scene)
{

}

Entity::~Entity()
{

}

void Entity::loadDataInputFile(DataInput &dataInput)
{
    string filename = dataInput.DATA_DIR + "input.txt";
    ifstream in;
    in.open(filename);
    if (!in.good()) {
        cout << "Cannot read " << filename << endl;
        return;
    }
    cout << "Loading " << filename << endl;

    string line;
    string currentEntity = "";
    while (true) {
        getline(in, line);
        if (in.eof()) {
            break;
        }

        // skip empty or commented lines
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // parse lines
        string key, value;
        stringstream ss(line);
        ss >> key;
        if (key.compare("ENTITY") == 0) {
            ss >> currentEntity;
            ss >> speed;
        }
        else if (currentEntity.compare(dataInput.entityType) == 0) {
            // this line is intended for this type of Entity
            if (key.compare("TEXTURE") == 0) {
                ss >> value;
                dataInput.textureData.push_back(value);
            }
            else if (key.compare("MESH") == 0) {
                vector<string> mesh;
                ss >> value;
                mesh.push_back(value); // obj filename
                ss >> value;
                mesh.push_back(value); // skin filename
                ss >> value;
                mesh.push_back(value); // texture filename
                dataInput.meshData.push_back(mesh);
            }
            else if (key.compare("SKELETON") == 0) {
                ss >> value;
                dataInput.skeletonData.push_back(value);
            }
            else {
                cout << "Unknown key word: " << key << endl;
            }
        }
    }

    in.close();
}

void Entity::loadSkeletonData(const DataInput &dataInput)
{
    for (int i = 0; i < dataInput.skeletonData.size(); i++) {
        string filename = dataInput.DATA_DIR + dataInput.skeletonData[i];
        ifstream in;
        in.open(filename);
        if (!in.good()) {
            cout << "Cannot read " << filename << endl;
            return;
        }
        cout << "Loading " << filename << endl;

        string line;
        vector< vector<glm::mat4> > animation;
        bool countsLoaded = false;
        bool bindPoseLoaded = false;
        int frameCount, boneCount;
        int currentFrame = 0;
        while (1) {
            getline(in, line);
            if (in.eof()) {
                break;
            }
            if (line.empty()) {
                continue;
            }
            // Skip comments
            if (line.at(0) == '#') {
                continue;
            }
            // Parse lines
            stringstream ss(line);
            if (!countsLoaded) { // load frameCount & boneCount
                ss >> frameCount >> boneCount;
                countsLoaded = true;
            }
            else if (!bindPoseLoaded) {
                for (int bone = 0; bone < boneCount; bone++) {
                    // load quaternion
                    float qx, qy, qz, qw;
                    ss >> qx >> qy >> qz >> qw;
                    glm::quat q(qw, qx, qy, qz);

                    // load translation vector
                    float vx, vy, vz;
                    ss >> vx >> vy >> vz;
                    glm::vec3 v(vx, vy, vz);

                    glm::mat4 E = glm::mat4_cast(q);
                    E[3] = glm::vec4(v, 1.0f);

                    bindPose.push_back(E);
                }

                bindPoseLoaded = true;
            }
            else { // load frame data
                //frames[i].push_back(vector<glm::mat4>());
                animation.push_back(vector<glm::mat4>());

                for (int bone = 0; bone < boneCount; bone++) {
                    // load quaternion
                    float qx, qy, qz, qw;
                    ss >> qx >> qy >> qz >> qw;
                    glm::quat q(qw, qx, qy, qz);

                    // load translation vector
                    float vx, vy, vz;
                    ss >> vx >> vy >> vz;
                    glm::vec3 v(vx, vy, vz);

                    glm::mat4 E = glm::mat4_cast(q);
                    E[3] = glm::vec4(v, 1.0f);

                    //frames[i][currentFrame].push_back(E);
                    animation[currentFrame].push_back(E);
                }

                currentFrame++;
            }
        }
        in.close();
        frames.push_back(animation);
    }
}

void Entity::generatePath()
{
    pg->updateStart(pos);
    pg->updateGoal(goal);

    // keep trying A* search until a pathway becomes available
    path = pg->findPath();
    while (path.size() == 0) {
        cout << "Regenerating PathGraph to find valid path." << endl;
        pg->regenerate(pos, goal);
        path = pg->findPath();
    }

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

//void Entity::setSkinInfo(SkinInfo &s)
//{
//    skins = s.skins;
//    frames = s.frames;
//    bindPose = s.bindPose;
//    textureMap = s.textureMap;
//}

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

    // update entity based on current state
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
            glm::vec3 pos_flat(pos.x, 0.0f, pos.z);
            glm::vec3 oldPos_flat(oldPos.x, 0.0f, oldPos.z);
            if (pos_flat != oldPos_flat) {
                rot = glm::inverse(glm::lookAt(pos_flat, oldPos_flat, glm::vec3(0.0f, 1.0f, 0.0f)));
            }

            break;
    }

    // update current frame
    double fps = 30;
    int frameCount = frames[state].size();
    currentFrame = ((int)floor(t*fps)) % frameCount;
}

void Entity::draw(shared_ptr<MatrixStack> P, shared_ptr<MatrixStack> MV, bool isSelected, bool drawPG, bool drawPath)
{
    // draw skin
    MV->pushMatrix();
    //MV->rotate(rot, glm::vec3(0.0f, 1.0f, 0.0f));
    pos.y = scene->getAltitude(pos);
    MV->translate(pos);
    MV->scale((isSelected ? 0.06f : 0.05f));
    //MV->scale(0.05f);
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
    pg->draw(P, MV, path, isSelected, drawPG, drawPath);
}
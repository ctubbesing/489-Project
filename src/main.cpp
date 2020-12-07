#include <iostream>
#include <fstream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#ifndef M_PI
#define M_PI (float)3.14159265358979323846
#endif

#include "GLSL.h"
#include "Program.h"
#include "Camera.h"
#include "MatrixStack.h"
#include "Shape.h"
#include "Scene.h"
#include "Terrain.h"
#include "Entity.h"
#include "ShapeSkin.h"
#include "Texture.h"
#include "PathGraph.h"

using namespace std;

//class DataInput
//{
//public:
//    vector<string> textureData;
//    vector< vector<string> > meshData;
//    vector<string> skeletonData;
//};

DataInput dataInput;

bool keyToggles[256] = {false};

GLFWwindow *window; // Main application window
string RESOURCE_DIR = "";
string DATA_DIR = "";
float TERRAIN_SIZE = 100.0f;
int TERRAIN_CELLS = 50; // 100;
int PG_UNITS_PER_NODE = 20;
bool DO_FLAT_TERRAIN = false;

shared_ptr<Camera> camera = NULL;
shared_ptr<Program> progSimple = NULL;
shared_ptr<Program> progShapes = NULL;
shared_ptr<Program> progTerrain = NULL;
shared_ptr<Program> progSkin = NULL;
shared_ptr<Scene> scene = NULL;
shared_ptr<Entity> selectedEnt = NULL;
//map< string, shared_ptr<Texture> > textureMap;
//vector< shared_ptr<ShapeSkin> > shapes;
//vector<glm::mat4> bindPose;
//vector< vector< vector<glm::mat4> > > frames;
double t, t0;
double tMult = 1.0;

///////////////////////////////
//shared_ptr<Shape> pmShape;
//glm::vec3 testSpot(0.0f);
//shared_ptr<Shape> eShape;
//shared_ptr<PathGraph> pg;
//vector< shared_ptr<PathNode> > pgPath;
///////////////////////////////

static void error_callback(int error, const char *description)
{
    cerr << description << endl;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

float randFloat(float l, float h)
{
    float r = rand() / (float)RAND_MAX;
    return (1.0f - r) * l + r * h;
}

static void char_callback(GLFWwindow *window, unsigned int key)
{
    keyToggles[key] = !keyToggles[key];
    bool badPos, badGoal;
    switch (key) {
        case (unsigned)'p':
            selectedEnt->regenPG();
            break;
        case (unsigned)'s':
            //pos = glm::vec3(-45.156f, 0.0f, 43.152f);
            glm::vec3 pos = glm::vec3(randFloat(-TERRAIN_SIZE / 2, TERRAIN_SIZE / 2), 0.0f, randFloat(-TERRAIN_SIZE / 2, TERRAIN_SIZE / 2));
            badPos = true;
            while (badPos) {
                pos = glm::vec3(randFloat(-TERRAIN_SIZE / 2, TERRAIN_SIZE / 2), 0.0f, randFloat(-TERRAIN_SIZE / 2, TERRAIN_SIZE / 2));
                badPos = scene->isObstacle(pos);
            }
            selectedEnt->setPos(pos);
            break;
        case (unsigned)'g':
            //pos = glm::vec3(-6.227f, 0.0f, -40.561f);
            badGoal = true;
            glm::vec3 goal;
            while (badGoal) {
                goal = glm::vec3(randFloat(-TERRAIN_SIZE / 2, TERRAIN_SIZE / 2), 0.0f, randFloat(-TERRAIN_SIZE / 2, TERRAIN_SIZE / 2));
                badGoal = scene->isObstacle(goal);
            }
            //glm::vec3 goal = glm::vec3(randFloat(-TERRAIN_SIZE / 2, TERRAIN_SIZE / 2), 0.0f, randFloat(-TERRAIN_SIZE / 2, TERRAIN_SIZE / 2));
            selectedEnt->setGoal(goal);
            break;
        case (unsigned)'x':
            tMult *= 2;
            //ent->speedUp();
            break;
        case (unsigned)'X':
            tMult /= 2;
            //ent->resetSpeed();
            break;
        //case (unsigned)'/':
        //    testSpot.x = randFloat(-TERRAIN_SIZE / 2, TERRAIN_SIZE / 2);
        //    testSpot.z = randFloat(-TERRAIN_SIZE / 2, TERRAIN_SIZE / 2);
        //    //cout << "testSpot location:" << endl << "    x = " << testSpot.x << endl << "    z = " << testSpot.z << endl;

        //    testSpot.y = scene->getAltitude(testSpot);
        //    if (scene->isObstacle(testSpot)) {
        //        cout << "Obstacle!!!" << endl;
        //    }
        //    break;
        //case (unsigned)'f':
        //    pgPath = pg->findPath();
        //    break;
        //case (unsigned)'a':
        //    cout << "pgPath drawing " << (keyToggles[(unsigned)'a'] ? "on." : "off.");
        //    break;
    }
}

static void cursor_position_callback(GLFWwindow *window, double xmouse, double ymouse)
{
    int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    if (state == GLFW_PRESS) {
        camera->mouseMoved((float)xmouse, (float)ymouse);
    }
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    // get the current mouse position
    double xmouse, ymouse;
    glfwGetCursorPos(window, &xmouse, &ymouse);
    // get the current window size
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    if (action == GLFW_PRESS) {
        bool shift = mods & GLFW_MOD_SHIFT;
        bool ctrl  = mods & GLFW_MOD_CONTROL;
        bool alt   = mods & GLFW_MOD_ALT;
        camera->mouseClicked((float)xmouse, (float)ymouse, shift, ctrl, alt);
    }
}

//void loadSkeletonData()
//{
//    for (int i = 0; i < dataInput.skeletonData.size(); i++) {
//        string filename = DATA_DIR + dataInput.skeletonData[i];
//        ifstream in;
//        in.open(filename);
//        if (!in.good()) {
//            cout << "Cannot read " << filename << endl;
//            return;
//        }
//        cout << "Loading " << filename << endl;
//
//        string line;
//        vector< vector<glm::mat4> > animation;
//        bool countsLoaded = false;
//        bool bindPoseLoaded = false;
//        int frameCount, boneCount;
//        int currentFrame = 0;
//        while (1) {
//            getline(in, line);
//            if (in.eof()) {
//                break;
//            }
//            if (line.empty()) {
//                continue;
//            }
//            // Skip comments
//            if (line.at(0) == '#') {
//                continue;
//            }
//            // Parse lines
//            stringstream ss(line);
//            if (!countsLoaded) { // load frameCount & boneCount
//                ss >> frameCount >> boneCount;
//                countsLoaded = true;
//            }
//            else if (!bindPoseLoaded) {
//                for (int bone = 0; bone < boneCount; bone++) {
//                    // load quaternion
//                    float qx, qy, qz, qw;
//                    ss >> qx >> qy >> qz >> qw;
//                    glm::quat q(qw, qx, qy, qz);
//
//                    // load translation vector
//                    float vx, vy, vz;
//                    ss >> vx >> vy >> vz;
//                    glm::vec3 v(vx, vy, vz);
//
//                    glm::mat4 E = glm::mat4_cast(q);
//                    E[3] = glm::vec4(v, 1.0f);
//
//                    bindPose.push_back(E);
//                }
//
//                bindPoseLoaded = true;
//            }
//            else { // load frame data
//                //frames[i].push_back(vector<glm::mat4>());
//                animation.push_back(vector<glm::mat4>());
//
//                for (int bone = 0; bone < boneCount; bone++) {
//                    // load quaternion
//                    float qx, qy, qz, qw;
//                    ss >> qx >> qy >> qz >> qw;
//                    glm::quat q(qw, qx, qy, qz);
//
//                    // load translation vector
//                    float vx, vy, vz;
//                    ss >> vx >> vy >> vz;
//                    glm::vec3 v(vx, vy, vz);
//
//                    glm::mat4 E = glm::mat4_cast(q);
//                    E[3] = glm::vec4(v, 1.0f);
//
//                    //frames[i][currentFrame].push_back(E);
//                    animation[currentFrame].push_back(E);
//                }
//
//                currentFrame++;
//            }
//        }
//        in.close();
//        frames.push_back(animation);
//    }
//}

static void init()
{
    keyToggles[(unsigned)'c'] = true;

    camera = make_shared<Camera>();
    camera->setInitDistance(40.0f);

    /*////////////////////////////////////////////////////temp skin hard coding
    //DATA_DIR = "../data/";
    auto skin = make_shared<ShapeSkin>();
    shapes.push_back(skin);
    skin->setTextureMatrixType("bigvegas_Walking_newVegas_Elvis_BodyGeo.obj");
    skin->loadMesh(DATA_DIR + "bigvegas_Walking_newVegas_Elvis_BodyGeo.obj");
    skin->loadAttachment(DATA_DIR + "bigvegas_Walking_newVegas_Elvis_BodyGeo_skin.txt");
    skin->setTextureFilename("file1.jpg");

    // For skinned shape, CPU/GPU
    progSkin = make_shared<Program>();
    progSkin->setShaderNames(RESOURCE_DIR + "skin_vert.glsl", RESOURCE_DIR + "skin_frag.glsl");
    progSkin->setVerbose(true);
    progSkin->init();
    progSkin->addAttribute("aPos");
    progSkin->addAttribute("aNor");
    progSkin->addAttribute("aTex");
    progSkin->addUniform("P");
    progSkin->addUniform("MV");
    progSkin->addUniform("ka");
    progSkin->addUniform("ks");
    progSkin->addUniform("s");
    progSkin->addUniform("kdTex");
    progSkin->addUniform("T");

    // Bind the texture to unit 1.
    int unit = 1;
    progSkin->bind();
    glUniform1i(progSkin->getUniform("kdTex"), unit);
    progSkin->unbind();

    string filename = "file1.jpg";
    auto textureKd = make_shared<Texture>();
    textureMap[filename] = textureKd;
    textureKd->setFilename(DATA_DIR + filename);
    textureKd->init();
    textureKd->setUnit(unit); // Bind to unit 1
    textureKd->setWrapModes(GL_REPEAT, GL_REPEAT);

    shapes[0]->init();
    ////////////////////////////////////////////////////*/

    //// Create skin shapes
    //for (const auto &mesh : dataInput.meshData) {
    //    auto shape = make_shared<ShapeSkin>();
    //    shapes.push_back(shape);
    //    shape->setTextureMatrixType(mesh[0]);
    //    shape->loadMesh(DATA_DIR + mesh[0]);
    //    shape->loadAttachment(DATA_DIR + mesh[1]);
    //    shape->setTextureFilename(mesh[2]);
    //}
    
    //loadSkeletonData();
    
    // simple program
    progSimple = make_shared<Program>();
    progSimple->setShaderNames(RESOURCE_DIR + "simple_vert.glsl", RESOURCE_DIR + "simple_frag.glsl");
    progSimple->setVerbose(true);
    progSimple->init();
    progSimple->addUniform("P");
    progSimple->addUniform("MV");

    // for drawing PathMarkers & other non-textured shapes
    progShapes = make_shared<Program>();
    progShapes->setShaderNames(RESOURCE_DIR + "shape_phong_vert.glsl", RESOURCE_DIR + "shape_phong_frag.glsl");
    progShapes->setVerbose(true);
    progShapes->init();
    progShapes->addUniform("P");
    progShapes->addUniform("MV");
    progShapes->addUniform("lightPos");
    progShapes->addUniform("ka");
    progShapes->addUniform("kd");
    progShapes->addUniform("ks");
    progShapes->addUniform("s");
    progShapes->addAttribute("aPos");
    progShapes->addAttribute("aNor");
    progShapes->setVerbose(false);

    // for drawing Terrain
    progTerrain = make_shared<Program>();
    progTerrain->setShaderNames(RESOURCE_DIR + "terrain_phong_vert.glsl", RESOURCE_DIR + "terrain_phong_frag.glsl");
    progTerrain->setVerbose(true);
    progTerrain->init();
    progTerrain->addUniform("P");
    progTerrain->addUniform("MV");
    progTerrain->addUniform("lightPos");
    progTerrain->addUniform("ka");
    progTerrain->addUniform("kd");
    progTerrain->addUniform("ks");
    progTerrain->addUniform("s");
    progTerrain->addAttribute("aPos");
    progTerrain->addAttribute("aNor");
    progTerrain->setVerbose(true);

    // for drawing skinned characters
    progSkin = make_shared<Program>();
    progSkin->setShaderNames(RESOURCE_DIR + "skin_vert.glsl", RESOURCE_DIR + "skin_frag.glsl");
    progSkin->setVerbose(true);
    progSkin->init();
    progSkin->addAttribute("aPos");
    progSkin->addAttribute("aNor");
    progSkin->addAttribute("aTex");
    progSkin->addUniform("P");
    progSkin->addUniform("MV");
    progSkin->addUniform("ka");
    progSkin->addUniform("ks");
    progSkin->addUniform("s");
    progSkin->addUniform("kdTex");
    progSkin->addUniform("T");

    //for (auto shape : shapes) {
    //    shape->init();
    //}

    /////////////////////////////////////////////////////////////////////
    pmShape = make_shared<Shape>();
    //shared_ptr<Shape> pmShape = make_shared<Shape>();
    pmShape->setProgram(progShapes);
    pmShape->loadMesh(DATA_DIR + "marker2.obj");
    pmShape->scale(1.5f);
    pmShape->init();

    //shared_ptr<Shape> eShape = make_shared<Shape>();
    //eShape->setProgram(progShapes);
    //eShape->loadMesh(DATA_DIR + "sphere2.obj");
    //eShape->scale(1.5f);
    //eShape->init();

    //pg = make_shared<PathGraph>(scene, PG_UNITS_PER_NODE);
    //pg->setSimpleProgram(progSimple);
    //pg->setShapeProgram(progShapes);
    //pg->setShape(pmShape);


    //pgPath = vector< shared_ptr<PathNode> >();
    //pgPath.push_back(make_shared<PathNode>(glm::vec3(25, 0, 25)));
    //pgPath.push_back(make_shared<PathNode>(glm::vec3(-50, 0, 0)));
    //pgPath.push_back(make_shared<PathNode>(glm::vec3(-50, 0, 50)));
    //pgPath.push_back(make_shared<PathNode>(glm::vec3(0, 0, 0)));

    /////////////////////////////////////////////////////////////////////

    // initialize scene
    scene = make_shared<Scene>(TERRAIN_SIZE, TERRAIN_CELLS, DO_FLAT_TERRAIN, PG_UNITS_PER_NODE, DATA_DIR);
    scene->setProgSimple(progSimple);
    scene->setProgShapes(progShapes);
    scene->setProgSkin(progSkin);
    scene->setProgTerrain(progTerrain);
    scene->init();

    //// Bind the texture to unit 1.
    //int unit = 1;
    //progSkin->bind();
    //glUniform1i(progSkin->getUniform("kdTex"), unit);
    //progSkin->unbind();

    //for (const auto &filename : dataInput.textureData) {
    //    auto textureKd = make_shared<Texture>();
    //    textureMap[filename] = textureKd;
    //    textureKd->setFilename(DATA_DIR + filename);
    //    textureKd->init();
    //    textureKd->setUnit(unit); // Bind to unit 1
    //    textureKd->setWrapModes(GL_REPEAT, GL_REPEAT);
    //}

    // initialize entity
    //ent = make_shared<Entity>(glm::vec3(0.0f), scene, TERRAIN_SIZE, PG_UNITS_PER_NODE);
    //ent->setSkinProgram(progShapes);////////////////////////////////////////////////////////////////////////////////////temp till shapeskin is better
    //ent->setSkinProgram(progSkin);

    //vector< vector< vector< glm::mat4 > > > f;
    //f.push_back(frames);
    //SkinInfo s(shapes, frames, bindPose, textureMap);
    //ent->setSkinInfo(s);
    ////ent->setSkin(eShape);//////////////////////////////////
    //ent->setPGProgs(progSimple, progShapes);
    //ent->setPGShape(pmShape);

    //scene->addEntity(ent);

    
    // set background color
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    // enable z-buffer test
    glEnable(GL_DEPTH_TEST);
    // enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    
    // initialize time
    glfwSetTime(0.0);
    
    /////////////////////////////////////////////////////////////////////////////
    //testSpot.y = scene->getAltitude(testSpot);
    /////////////////////////////////////////////////////////////////////////////
    
    GLSL::checkError(GET_FILE_LINE);
}

void render()
{
    // update time
    double t1 = glfwGetTime();
    float dt = tMult * (t1 - t0);
    if (!keyToggles[(unsigned)' ']) {
        t += dt;
    }
    t0 = t1;

    // get current frame buffer size
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    // use the window size for camera
    glfwGetWindowSize(window, &width, &height);
    camera->setAspect((float)width / (float)height);

    // Clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (keyToggles[(unsigned)'c']) {
        glEnable(GL_CULL_FACE);
    }
    else {
        glDisable(GL_CULL_FACE);
    }
    if (keyToggles[(unsigned)'z']) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    shared_ptr<MatrixStack> P = make_shared<MatrixStack>();
    shared_ptr<MatrixStack> MV = make_shared<MatrixStack>();
    
    // apply camera transforms
    P->pushMatrix();
    camera->applyProjectionMatrix(P);
    MV->pushMatrix();
    camera->applyViewMatrix(MV);
    
    // draw axes
    progSimple->bind();
    glUniformMatrix4fv(progSimple->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
    MV->pushMatrix();
    //MV->translate(glm::vec3(0.0f, 10.0f, 0.0f));
    glUniformMatrix4fv(progSimple->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
    float len = 2.0f;
    glLineWidth(2);
    glBegin(GL_LINES);
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(len, 0.0f, 0.0f);
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, len, 0.0f);
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, len);
    glEnd();
    glLineWidth(1);
    MV->popMatrix();
    progSimple->unbind();

    // Draw grid
    progSimple->bind();
    glUniformMatrix4fv(progSimple->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
    glUniformMatrix4fv(progSimple->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
    int n = ceil(TERRAIN_SIZE / PG_UNITS_PER_NODE);
    float gridSizeHalf = TERRAIN_SIZE / 2;
    int gridNx = n + 1;
    int gridNz = n + 1;
    //float gridSizeHalf = 50.0f;
    //int gridNx = 11;
    //int gridNz = 11;
    glLineWidth(1);
    glColor3f(0.8f, 0.8f, 0.8f);
    glBegin(GL_LINES);
    for (int i = 0; i < gridNx; ++i) {
        float alpha = i / (gridNx - 1.0f);
        float x = (1.0f - alpha) * (-gridSizeHalf) + alpha * gridSizeHalf;///////////////////////////////////////
        glVertex3f(x, 0, -gridSizeHalf);
        glVertex3f(x, 0, gridSizeHalf);
    }
    for (int i = 0; i < gridNz; ++i) {
        float alpha = i / (gridNz - 1.0f);
        float z = (1.0f - alpha) * (-gridSizeHalf) + alpha * gridSizeHalf;
        glVertex3f(-gridSizeHalf, 0, z);
        glVertex3f(gridSizeHalf, 0, z);
    }
    glEnd();
    progSimple->unbind();

    //// do shape at terrain vertices
    //if (keyToggles[(unsigned)'v']) {
    //    progShapes->bind();

    //    glUniform3f(progShapes->getUniform("kd"), 0.2f, 0.5f, 0.6f);
    //    glUniform3f(progShapes->getUniform("ka"), 0.02f, 0.05f, 0.06f);
    //    glUniformMatrix4fv(progShapes->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));

    //    MV->pushMatrix();
    //    //MV->translate(glm::vec3(-0.75f, 0.0f, -0.75f));
    //    //MV->rotate(t, 0.0f, 1.0f, 0.0f);

    //    shared_ptr terrain = scene->getTerrain();
    //    for (int i = 0; i < TERRAIN_CELLS + 1; i++) {
    //        for (int j = 0; j < TERRAIN_CELLS + 1; j++) {
    //            MV->pushMatrix();
    //            MV->translate(terrain->getPoint(i, j));
    //            MV->rotate(t, 0.0f, 1.0f, 0.0f);
    //            MV->translate(glm::vec3(-0.75f, 0.0f, -0.75f));
    //            glUniformMatrix4fv(progShapes->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
    //            pmShape->draw();
    //            MV->popMatrix();
    //        }
    //    }

    //    ////////////////////////////////////////////////////////////////
    //    //MV->pushMatrix();
    //    //MV->translate(glm::vec3(2.5f, 0.0f, 2.5f));
    //    //MV->rotate(t, 0.0f, 1.0f, 0.0f);
    //    //MV->translate(glm::vec3(-0.75f, 0.0f, -0.75f));
    //    //glUniformMatrix4fv(progShapes->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
    //    //pmShape->draw();
    //    //MV->popMatrix();
    //    ////////////////////////////////////////////////////////////////

    //    MV->popMatrix();
    //    progShapes->unbind();
    //}

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //progShapes->bind();

    //glUniform3f(progShapes->getUniform("kd"), 0.6f, 0.2f, 0.5f);
    //glUniform3f(progShapes->getUniform("ka"), 0.06f, 0.02f, 0.05f);
    //glUniformMatrix4fv(progShapes->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));

    //MV->pushMatrix();
    ////MV->translate(glm::vec3(-0.75f, 0.0f, -0.75f));
    ////MV->rotate(t, 0.0f, 1.0f, 0.0f);

    ////testSpot.y = scene->getAltitude(testSpot);

    //MV->pushMatrix();
    //MV->translate(testSpot);
    //MV->rotate(t, 0.0f, 1.0f, 0.0f);
    //MV->translate(glm::vec3(-0.75f, 0.0f, -0.75f));
    //glUniformMatrix4fv(progShapes->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
    //pmShape->draw();
    //MV->popMatrix();
    //MV->popMatrix();
    //progShapes->unbind();
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // draw terrain
    //progTerrain->bind();
    //glUniformMatrix4fv(progTerrain->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
    //MV->pushMatrix();
    scene->draw(P, MV, t, keyToggles[(unsigned)'e'], keyToggles[(unsigned)'E']);
    //MV->popMatrix();
    //progTerrain->unbind();
    
    //// draw pg
    //if (keyToggles[(unsigned)'a']) {
    //    pg->draw(P, MV, pgPath);
    //}
    //else {
    //    pg->draw(P, MV);
    //}

    //// draw characters
    //double fps = 30;
    //int frameCount = frames.size();
    //int frame = ((int)floor(t*fps)) % frameCount;
    //for (int i = 0; i < 1; i++) {
    //    MV->pushMatrix();
    //    MV->translate(i * 10.0f, 0, 0);
    //    MV->rotate(i * M_PI / 2, 0.0f, 1.0f, 0.0f);

    //    for (const auto &shape : shapes) {
    //        MV->pushMatrix();

    //        progSkin->bind();
    //        textureMap[shape->getTextureFilename()]->bind(progSkin->getUniform("kdTex"));
    //        glLineWidth(1.0f); // for wireframe
    //        MV->scale(0.05f);
    //        glUniformMatrix4fv(progSkin->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
    //        glUniformMatrix4fv(progSkin->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
    //        glUniform3f(progSkin->getUniform("ka"), 0.1f, 0.1f, 0.1f);
    //        glUniform3f(progSkin->getUniform("ks"), 0.1f, 0.1f, 0.1f);
    //        glUniform1f(progSkin->getUniform("s"), 200.0f);
    //        shape->setProgram(progSkin);
    //        shape->update(bindPose, frames[frame]);
    //        shape->draw();
    //        progSkin->unbind();

    //        MV->popMatrix();
    //    }
    //    MV->popMatrix();
    //}

    // pop matrix stacks
    MV->popMatrix();
    P->popMatrix();
    
    GLSL::checkError(GET_FILE_LINE);
}

//void loadDataInputFile()
//{
//    string filename = DATA_DIR + "input.txt";
//    ifstream in;
//    in.open(filename);
//    if (!in.good()) {
//        cout << "Cannot read " << filename << endl;
//        return;
//    }
//    cout << "Loading " << filename << endl;
//
//    string line;
//    while (true) {
//        getline(in, line);
//        if (in.eof()) {
//            break;
//        }
//
//        // skip empty or commented lines
//        if (line.empty() || line[0] == '#') {
//            continue;
//        }
//
//        // parse lines
//        string key, value;
//        stringstream ss(line);
//        ss >> key;
//        if (key.compare("TEXTURE") == 0) {
//            ss >> value;
//            dataInput.textureData.push_back(value);
//        }
//        else if (key.compare("MESH") == 0) {
//            vector<string> mesh;
//            ss >> value;
//            mesh.push_back(value); // obj filename
//            ss >> value;
//            mesh.push_back(value); // skin filename
//            ss >> value;
//            mesh.push_back(value); // texture filename
//            dataInput.meshData.push_back(mesh);
//        }
//        else if (key.compare("SKELETON") == 0) {
//            ss >> value;
//            dataInput.skeletonData.push_back(value);
//        }
//        else {
//            cout << "Unknown key word: " << key << endl;
//        }
//    }
//
//    in.close();
//}

int main(int argc, char **argv)
{
    if (argc < 3) {
        cout << "Usage: A3 <SHADER DIR> <DATA DIR>" << endl;
        return 0;
    }
    RESOURCE_DIR = argv[1] + string("/");
    DATA_DIR = argv[2] + string("/");
    //loadDataInputFile();
	
	// Set error callback.
	glfwSetErrorCallback(error_callback);
	// Initialize the library.
	if(!glfwInit()) {
		return -1;
	}
	// Create a windowed mode window and its OpenGL context.
	window = glfwCreateWindow(1200, 900, "Connor Tubbesing", NULL, NULL);
	if(!window) {
		glfwTerminate();
		return -1;
	}
	// Make the window's context current.
	glfwMakeContextCurrent(window);
	// Initialize GLEW.
	glewExperimental = true;
	if(glewInit() != GLEW_OK) {
		cerr << "Failed to initialize GLEW" << endl;
		return -1;
	}
	glGetError(); // A bug in glewInit() causes an error that we can safely ignore.
	cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
	// Set vsync.
	glfwSwapInterval(1);
	// Set keyboard callback.
	glfwSetKeyCallback(window, key_callback);
	// Set char callback.
	glfwSetCharCallback(window, char_callback);
	// Set cursor position callback.
	glfwSetCursorPosCallback(window, cursor_position_callback);
	// Set mouse button callback.
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	// Initialize scene.
	init();
	// Loop until the user closes the window.
	while(!glfwWindowShouldClose(window) && !keyToggles[(unsigned)'q']) {
		// Render scene.
		render();
		// Swap front and back buffers.
		glfwSwapBuffers(window);
		// Poll for and process events.
		glfwPollEvents();
	}
	// Quit program.
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;}

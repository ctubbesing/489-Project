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

bool keyToggles[256] = {false};

GLFWwindow *window; // Main application window
string RESOURCE_DIR = "";
string DATA_DIR = "";
float TERRAIN_SIZE = 100.0f;
int TERRAIN_CELLS = 50;
bool DO_FLAT_TERRAIN = false;

shared_ptr<Camera> camera = NULL;
shared_ptr<Program> progSimple = NULL;
shared_ptr<Program> progShapes = NULL;
shared_ptr<Program> progTerrain = NULL;
shared_ptr<Program> progSkin = NULL;
shared_ptr<Scene> scene = NULL;
shared_ptr<Entity> selectedEnt = NULL;
double t, t0;
double tMult = 1.0;

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
            glm::vec3 pos;
            badPos = true;
            while (badPos) {
                pos = glm::vec3(randFloat(-TERRAIN_SIZE / 2, TERRAIN_SIZE / 2), 0.0f, randFloat(-TERRAIN_SIZE / 2, TERRAIN_SIZE / 2));
                badPos = scene->isObstacle(pos);
            }
            selectedEnt->setPos(pos);
            break;

        case (unsigned)'g':
            badGoal = true;
            glm::vec3 goal;
            while (badGoal) {
                goal = glm::vec3(randFloat(-TERRAIN_SIZE / 2, TERRAIN_SIZE / 2), 0.0f, randFloat(-TERRAIN_SIZE / 2, TERRAIN_SIZE / 2));
                badGoal = scene->isObstacle(goal);
            }
            selectedEnt->setGoal(goal);
            break;

        case (unsigned)'d':
            selectedEnt = scene->addEntity();
            break;

        case (unsigned)'D':
            selectedEnt = scene->deleteEntity();
            break;

        case (unsigned)'t':
            selectedEnt = scene->selectEntity();
            break;

        case (unsigned)'x':
            tMult *= 2;
            break;

        case (unsigned)'X':
            tMult /= 2;
            break;
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

static void init()
{
    keyToggles[(unsigned)'c'] = true;

    camera = make_shared<Camera>();
    camera->setInitDistance(40.0f);
    
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

    // initialize scene
    scene = make_shared<Scene>(TERRAIN_SIZE, TERRAIN_CELLS, DO_FLAT_TERRAIN, DATA_DIR);
    scene->setProgSimple(progSimple);
    scene->setProgShapes(progShapes);
    scene->setProgSkin(progSkin);
    scene->setProgTerrain(progTerrain);
    selectedEnt = scene->init();
    
    // set background color
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    // enable z-buffer test
    glEnable(GL_DEPTH_TEST);
    // enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // initialize time
    glfwSetTime(0.0);
    
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
    int n = ceil(TERRAIN_SIZE / 20);
    float gridSizeHalf = TERRAIN_SIZE / 2;
    int gridNx = n + 1;
    int gridNz = n + 1;
    glLineWidth(1);
    glColor3f(0.8f, 0.8f, 0.8f);
    glBegin(GL_LINES);
    for (int i = 0; i < gridNx; ++i) {
        float alpha = i / (gridNx - 1.0f);
        float x = (1.0f - alpha) * (-gridSizeHalf) + alpha * gridSizeHalf;
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

    // draw scene
    scene->draw(P, MV, t, keyToggles[(unsigned)'e'], keyToggles[(unsigned)'E']);

    // pop matrix stacks
    MV->popMatrix();
    P->popMatrix();
    
    GLSL::checkError(GET_FILE_LINE);
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        cout << "Usage: FinalProject.exe <SHADER DIR> <DATA DIR>" << endl;
        cout << "Using default \"../resources ../data\"" << endl;
        RESOURCE_DIR = string("../resources/");
        DATA_DIR = string("../data/");
    }
    else {
        RESOURCE_DIR = argv[1] + string("/");
        DATA_DIR = argv[2] + string("/");
    }

	// Set error callback.
	glfwSetErrorCallback(error_callback);
	// Initialize the library.
	if(!glfwInit()) {
		return -1;
	}
	// Create a windowed mode window and its OpenGL context.
	window = glfwCreateWindow(1200, 900, "Terrain Pathfinding Scene by Connor Tubbesing", NULL, NULL);
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
	return 0;
}

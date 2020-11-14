#include "PathMarker.h"
#include "Program.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/gtc/type_ptr.hpp>

using namespace std;

PathMarker::PathMarker()
{

}

PathMarker::~PathMarker()
{

}

//void PathMarker::draw(shared_ptr<MatrixStack> MV, shared_ptr<Program> prog)
//{
//    MV->pushMatrix();
//    MV->translate(glm::vec3(-0.75f, 0.0f, -0.75f));
//    glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
//    shape.draw();
//    MV->popMatrix();
//}

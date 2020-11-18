#include "PathGraph.h"
//#include "PathMarker.h"
#include "Scene.h"
#include "Program.h"
#include "MatrixStack.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


using namespace std;

PathGraph::PathGraph(shared_ptr<Scene> scene_) :
    scene(scene_)
{
    regenerate();
}

PathGraph::~PathGraph()
{

}

float PathGraph::randFloat(float l, float h)
{
    float r = rand() / (float)RAND_MAX;
    return (1.0f - r) * l + r * h;
}

void PathGraph::regenerate()
{
    nodes.clear();

    float edgeLength = scene->getSize();
    //int n = floor((edgeLength * edgeLength) / 8);
    int n = 10;
    nodes.reserve(n);

    for (int i = 0; i < n; i++) {
        glm::vec3 pos(i, 0.0f, 5 * i);
        //glm::vec3 pos(randFloat(-edgeLength / 2, edgeLength / 2), 0.0f, randFloat(-edgeLength / 2, edgeLength / 2));
        auto newNode = make_shared<PathNode>(pos);
        nodes.push_back(newNode);
    }
}

vector< shared_ptr<PathNode> > PathGraph::findPath(shared_ptr<PathNode> start, shared_ptr<PathNode> goal)
{
    return vector< shared_ptr<PathNode> >();
}

void PathGraph::draw(shared_ptr<MatrixStack> P, shared_ptr<MatrixStack> MV, vector< shared_ptr<PathNode> > path)
{
    // assert path in nodes?

    //if (path.size() > 0) {
    //    // ...
    //}

    /*glUniformMatrix4fv(shapeProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));*/

    // draw each node shape
    shapeProg->bind();

    glUniform3f(shapeProg->getUniform("kd"), 0.2f, 0.5f, 0.6f);
    glUniform3f(shapeProg->getUniform("ka"), 0.5f, 0.05f, 0.06f);
    glUniformMatrix4fv(shapeProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));

    for (auto node : nodes) {
        MV->pushMatrix();

        MV->translate(node->pos);
        glUniformMatrix4fv(shapeProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));

        PmShape->draw();

        MV->popMatrix();
    }

    shapeProg->unbind();

    // draw each edge
    
}

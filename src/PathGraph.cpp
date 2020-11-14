#include "PathGraph.h"
//#include "PathMarker.h"
#include "Program.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


using namespace std;

PathGraph::PathGraph()
{
    nodes = vector< shared_ptr<PathNode> >();
}

PathGraph::~PathGraph()
{

}

void PathGraph::regenerate(shared_ptr<Scene> scene)
{

}

vector< shared_ptr<PathNode> > PathGraph::findPath(shared_ptr<PathNode> start, shared_ptr<PathNode> goal)
{
    return vector< shared_ptr<PathNode> >();
}

void PathGraph::draw(shared_ptr<MatrixStack> MV, vector< shared_ptr<PathNode> > path)
{
    // assert path in nodes?

    //if (path.size() > 0) {
    //    // ...
    //}

    /*glUniformMatrix4fv(shapeProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));*/

    // draw each node shape
    //for (int i = 0; i < nodes.size(); i++) {
    //    nodes[i]->draw(MV, shapeProg);
    //}

    // draw each edge

}

#include "PathGraph.h"
//#include "PathMarker.h"
#include "Scene.h"
#include "Program.h"
#include "MatrixStack.h"

#include <iostream>

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
    int n = ceil(edgeLength / 5);
    //int n = edgeLength / 5;
    //nodes.reserve(n);

    cout << "PathGraph::regenerate(): edgeLength = " << edgeLength << endl;
    cout << "PathGraph::regenerate(): n = " << n << endl;

    float dx = edgeLength / n;
    float pos0 = -edgeLength / 2 + dx / 2;
    for (int i = 0; i < n; i++) {
        vector < shared_ptr<PathNode> > thisRow(n);
        
        for (int j = 0; j < n; j++) {
            // create new node
            float posX = dx * j;
            float posZ = dx * i;

            //float randX = randFloat(-dx / 2, dx / 2);
            //float randZ = randFloat(-dx / 2, dx / 2);
            posX += randFloat(-dx / 2, dx / 2);
            posZ += randFloat(-dx / 2, dx / 2);

            //glm::vec3 pos(pos0 + posX + randX, 0.0f, pos0 + posZ + randZ);
            glm::vec3 pos(pos0 + posX, 0.0f, pos0 + posZ);
            auto newNode = make_shared<PathNode>(pos);
            thisRow[j] = newNode;

            // link node to neighbors
            if (i > 0) {
                newNode->addNeighbor(nodes[i - 1][j]);
                if (j > 0) {
                    newNode->addNeighbor(nodes[i - 1][j - 1]);
                }
                if (j < n - 1) {
                    newNode->addNeighbor(nodes[i - 1][j + 1]);
                }
            }
            if (j > 0) {
                newNode->addNeighbor(thisRow[j - 1]);
            }
        }
        nodes.push_back(thisRow);
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

    //glUniform3f(shapeProg->getUniform("kd"), 0.2f, 0.5f, 0.6f);
    //glUniform3f(shapeProg->getUniform("ka"), 0.5f, 0.05f, 0.06f);
    glUniformMatrix4fv(shapeProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));

    float rW = 0.6f;
    float gW = 0.8f;
    float bW = 1.0f;
    for (int i = 0; i < nodes.size(); i++) {
        float randColor = sin(5*i) / 2 + 0.5f;
        randColor += sin(0.5*i) / 2 + 0.5f;
        randColor += abs(cos(i)) / 2 + 0.5f;
        randColor /= 3;
        glUniform3f(shapeProg->getUniform("ka"), rW*randColor, gW*randColor, bW*randColor);
        glUniform3f(shapeProg->getUniform("kd"), rW*randColor, gW*randColor, bW*randColor);
        auto nodeRow = nodes[i];
        for (auto node : nodeRow) {

            MV->pushMatrix();

            MV->translate(node->pos);
            MV->translate(glm::vec3(-0.75f, 0.0f, -0.75f));
            glUniformMatrix4fv(shapeProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
            PmShape->draw();

            MV->popMatrix();
        }
    }

    shapeProg->unbind();

    // draw each edge
    simpleProg->bind();

    glUniformMatrix4fv(simpleProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
    glUniformMatrix4fv(simpleProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));

    glLineWidth(2);
    glColor3f(0.4f, 0.4f, 0.8f);
    glBegin(GL_LINES);
    for (int i = 0; i < nodes.size(); ++i) {
        for (int j = 0; j < nodes[i].size(); j++) {
            //float alpha = i / (gridNx - 1.0f);
            //float x = (1.0f - alpha) * (-gridSizeHalf) + alpha * gridSizeHalf;

            auto node = nodes[i][j];
            glm::vec3 pos0 = node->pos;
            for (auto neighbor : node->neighbors) {
                glm::vec3 pos1 = neighbor->pos;

                glVertex3f(pos0.x, pos0.y, pos0.z);
                glVertex3f(pos1.x, pos1.y, pos1.z);
            }
        }
    }

    glEnd();

    // draw selected path
    if (path.size() > 0) {
        glLineWidth(10);
        glColor3f(0.8f, 0.4f, 0.4f);
        glBegin(GL_LINES);
        for (int i = 0; i < path.size() - 1; i++) {
            glm::vec3 pos0 = path[i]->pos;
            glm::vec3 pos1 = path[i + 1]->pos;

            glVertex3f(pos0.x, pos0.y, pos0.z);
            glVertex3f(pos1.x, pos1.y, pos1.z);
        }
        glEnd();
    }

    simpleProg->unbind();
}

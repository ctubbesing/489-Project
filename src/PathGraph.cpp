#include "PathGraph.h"
#include "Scene.h"
#include "Program.h"
#include "MatrixStack.h"

#include <iostream>
#include <queue>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

PathGraph::PathGraph()
{

}

PathGraph::PathGraph(const shared_ptr<Scene> _scene, ProgInfo progs, string DATA_DIR, int _unitsPerNode) :
    scene(_scene),
    edgeLength(_scene->getEdgeLength()),
    unitsPerNode(_unitsPerNode),
    start(make_shared<PathNode>(glm::vec3(0.0f))),
    goal(make_shared<PathNode>(glm::vec3(0.0f))),
    simpleProg(progs.progSimple),
    shapeProg(progs.progShapes)
{
    // load shape from file
    PmShape = make_shared<Shape>();
    PmShape->setProgram(progs.progShapes);
    PmShape->loadMesh(DATA_DIR + "marker2.obj");
    PmShape->scale(1.5f);
    PmShape->init();

    regenerate();
}

PathGraph::PathGraph(const PathGraph &pg) :
    scene(pg.scene),
    edgeLength(pg.edgeLength),
    unitsPerNode(pg.unitsPerNode),
    simpleProg(pg.simpleProg),
    shapeProg(pg.shapeProg),
    PmShape(pg.PmShape)
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

bool PathGraph::isClearPath(shared_ptr<PathNode> a, shared_ptr<PathNode> b)
{
    // determine whether path between two nodes is obstacle-free
    glm::vec3 a2b = b->pos - a->pos;
    float l = glm::length(a2b);

    int timesToCheck = 100;
    for (int i = 0; i < timesToCheck; i++) {
        float w = (float)i / timesToCheck;
        glm::vec3 p = a->pos + w * a2b;

        if (scene->isObstacle(p)) {
            return false;
        }
    }

    return true;
}

void PathGraph::regenerate()
{
    nodes.clear();

    int n = ceil(edgeLength / unitsPerNode);
    float dx = edgeLength / n;

    // set each node position to a new random value
    float pos0 = -edgeLength / 2 + dx / 2;
    for (int i = 0; i < n; i++) {
        vector < shared_ptr<PathNode> > thisRow(n);

        for (int j = 0; j < n; j++) {
            // create new node
            float posX = dx * j;
            float posZ = dx * i;
            float randErrMax = dx / 3;
            posX += randFloat(-randErrMax, randErrMax);
            posZ += randFloat(-randErrMax, randErrMax);

            glm::vec3 pos(pos0 + posX, 0.0f, pos0 + posZ);
            pos.y = scene->getAltitude(pos);

            auto newNode = make_shared<PathNode>(pos);
            thisRow[j] = newNode;

            // link node to neighbors
            shared_ptr<PathNode> neighbor;
            if (i > 0) {
                neighbor = nodes[i - 1][j];
                if (isClearPath(newNode, neighbor)) {
                    newNode->addNeighbor(neighbor);
                }

                if (j > 0) {
                    neighbor = nodes[i - 1][j - 1];
                    if (isClearPath(newNode, neighbor)) {
                        newNode->addNeighbor(neighbor);
                    }
                }
                if (j < n - 1) {
                    neighbor = nodes[i - 1][j + 1];
                    if (isClearPath(newNode, neighbor)) {
                        newNode->addNeighbor(neighbor);
                    }
                }
            }
            if (j > 0) {
                neighbor = thisRow[j - 1];
                if (isClearPath(newNode, neighbor)) {
                    newNode->addNeighbor(neighbor);
                }
            }
        }

        nodes.push_back(thisRow);
    }

    // remove nodes that are in obstacles
    for (auto nodeRow : nodes) {
        for (auto node : nodeRow) {
            if (scene->isObstacle(node->pos)) {
                node->clearNeighbors();
                node = NULL;
            }
        }
    }

    // update start and goal links
    if (start != NULL) {
        updateStart(start->pos);
    }
    if (goal != NULL) {
        updateGoal(goal->pos);
    }
}

void PathGraph::regenerate(glm::vec3 _start, glm::vec3 _goal)
{
    if (start == NULL) {
        start = make_shared<PathNode>(glm::vec3(0.0f));
    }
    if (goal == NULL) {
        goal = make_shared<PathNode>(glm::vec3(0.0f));
    }
    
    start->pos = _start;
    goal->pos = _goal;

    regenerate();
}

void PathGraph::updateStart(glm::vec3 pos)
{
    // update old start
    if (start != NULL) {
        start->clearNeighbors();

        pos.y = scene->getAltitude(pos);
        start->pos = pos;
    }
    else {
        start = make_shared<PathNode>(pos);
    }

    // link to new neighbors
    int n = ceil(edgeLength / unitsPerNode);
    float dx = edgeLength / n;

    int row = floor((pos.z + edgeLength / 2) / dx);
    int col = floor((pos.x + edgeLength / 2) / dx);

    shared_ptr<PathNode> mainNode = nodes[row][col];
    for (auto node : mainNode->neighbors) {
        if (isClearPath(start, node)) {
            start->addNeighbor(node);
        }
    }
    if (isClearPath(start, mainNode)) {
        start->addNeighbor(mainNode);
    }
}

void PathGraph::updateGoal(glm::vec3 pos)
{
    // update old goal
    if (goal != NULL) {
        goal->clearNeighbors();

        pos.y = scene->getAltitude(pos);
        goal->pos = pos;
    }
    else {
        goal = make_shared<PathNode>(pos);
    }

    // link to new neighbors
    int n = ceil(edgeLength / unitsPerNode);
    float dx = edgeLength / n;

    int row = floor((pos.z + edgeLength / 2) / dx);
    int col = floor((pos.x + edgeLength / 2) / dx);

    shared_ptr<PathNode> mainNode = nodes[row][col];
    for (auto node : mainNode->neighbors) {
        if (isClearPath(goal, node)) {
            goal->addNeighbor(node);
        }
    }
    goal->addNeighbor(mainNode);
    if (isClearPath(goal, mainNode)) {
        goal->addNeighbor(mainNode);
    }
}

// struct for priority queue to order elements
struct BranchCompare
{
    bool operator()(const shared_ptr<AStarBranch> &a, const shared_ptr<AStarBranch> &b)
    {
        return a->f > b->f;
    }
};

vector< glm::vec3 > PathGraph::findPath()
{
    // return empty path if start or goal is not available
    if (start == NULL || goal == NULL) {
        return vector<glm::vec3>();
    }

    // do A* search from start to goal
    priority_queue<shared_ptr<AStarBranch>, vector< shared_ptr<AStarBranch> >, BranchCompare> pq;
    shared_ptr<AStarBranch> currentBranch = make_shared<AStarBranch>(start);
    while (currentBranch->path.back() != goal) {
        // expand current branch
        for (auto node : currentBranch->path.back()->neighbors) {
            // avoid loops in the path
            bool isVisited = false;
            for (auto visitedNode : currentBranch->path) {
                if (node == visitedNode) {
                    isVisited = true;
                    break;
                }
            }

            if (!isVisited) {
                glm::vec3 dx(goal->pos - node->pos);
                float h = sqrt(dx.x * dx.x + dx.z * dx.z);
                shared_ptr<AStarBranch> newBranch = make_shared<AStarBranch>(currentBranch, node, h);
                pq.push(newBranch);
            }
        }

        // go to next branch in pq
        if (!pq.empty()) {
            currentBranch = pq.top();
            pq.pop();
        }
        else {
            break;
        }
    }

    if (currentBranch->path.back() != goal) {
        // no path from start to goal exists
        return vector<glm::vec3>();
    }

    // convert branch path to vector of glm::vec3
    vector<glm::vec3> finalPath;
    for (auto node : currentBranch->path) {
        finalPath.push_back(node->pos);
    }

    return finalPath;
}

void PathGraph::draw(shared_ptr<MatrixStack> P, shared_ptr<MatrixStack> MV, vector<glm::vec3> &path, bool isSelected, bool drawFullPG, bool drawPath)
{
    // --- draw normal nodes ---
    if (drawFullPG) {
        shapeProg->bind();

        glUniformMatrix4fv(shapeProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));

        float rW = 0.6f;
        float gW = 0.8f;
        float bW = 1.0f;
        for (int i = 0; i < nodes.size(); i++) {
            float randColor = sin(5 * i) / 2 + 0.5f;
            randColor += sin(0.5*i) / 2 + 0.5f;
            randColor += abs(cos(i)) / 2 + 0.5f;
            randColor /= 3;
            glUniform3f(shapeProg->getUniform("ka"), rW*randColor, gW*randColor, bW*randColor);
            glUniform3f(shapeProg->getUniform("kd"), rW*randColor, gW*randColor, bW*randColor);
            auto nodeRow = nodes[i];
            for (auto node : nodeRow) {
                if (node != NULL) {
                    for (glm::vec3 pathPos : path) {
                        if (node->pos == pathPos) {
                            glUniform3f(shapeProg->getUniform("ka"), 0.3f, 0.0f, 0.0f);
                            glUniform3f(shapeProg->getUniform("kd"), 0.9f, 0.2f, 0.2f);
                            break;
                        }
                        else {
                            glUniform3f(shapeProg->getUniform("ka"), rW*randColor, gW*randColor, bW*randColor);
                            glUniform3f(shapeProg->getUniform("kd"), rW*randColor, gW*randColor, bW*randColor);
                        }
                    }

                    MV->pushMatrix();

                    MV->translate(node->pos);
                    MV->scale(0.75f);
                    MV->translate(glm::vec3(-0.75f, 0.0f, -0.75f));
                    glUniformMatrix4fv(shapeProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
                    PmShape->draw();

                    MV->popMatrix();
                }
            }
        }

        shapeProg->unbind();
    }

    // --- draw each edge ---
    if (drawFullPG) {
        simpleProg->bind();

        glUniformMatrix4fv(simpleProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
        glUniformMatrix4fv(simpleProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));

        glLineWidth(2);
        glColor3f(0.4f, 0.4f, 0.8f);
        glBegin(GL_LINES);
        for (int i = 0; i < nodes.size(); ++i) {
            for (int j = 0; j < nodes[i].size(); j++) {
                auto node = nodes[i][j];
                if (node != NULL) {
                    glm::vec3 pos0 = node->pos;
                    for (auto neighbor : node->neighbors) {
                        glm::vec3 pos1 = neighbor->pos;

                        glVertex3f(pos0.x, pos0.y + 0.2f, pos0.z);
                        glVertex3f(pos1.x, pos1.y + 0.2f, pos1.z);
                    }
                }
            }
        }

        glEnd();
        simpleProg->unbind();
    }


    // --- draw start & goal ---
    // start
    if (start != NULL && drawPath) {
        // draw shape
        shapeProg->bind();
        glUniformMatrix4fv(shapeProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
        glUniform3f(shapeProg->getUniform("ka"), 0.5f, 0.5f, 0.0f);
        glUniform3f(shapeProg->getUniform("kd"), 0.7f, 0.7f, 0.0f);

        MV->pushMatrix();
        MV->translate(start->pos);
        MV->translate(glm::vec3(-0.75f, 0.0f, -0.75f));
        glUniformMatrix4fv(shapeProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
        PmShape->draw();
        MV->popMatrix();

        shapeProg->unbind();
    }
    
    // goal
    if (goal != NULL && isSelected) {
        // draw shape
        shapeProg->bind();
        glUniformMatrix4fv(shapeProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
        glUniform3f(shapeProg->getUniform("ka"), 0.0f, 0.5f, 0.0f);
        glUniform3f(shapeProg->getUniform("kd"), 0.0f, 0.7f, 0.0f);

        MV->pushMatrix();
        MV->translate(goal->pos);
        MV->translate(glm::vec3(-0.75f, 0.0f, -0.75f));
        glUniformMatrix4fv(shapeProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
        PmShape->draw();
        MV->popMatrix();

        shapeProg->unbind();
    }
    
    // --- draw provided path ---
    if (drawPath) {
        if (path.size() > 0) {
            simpleProg->bind();
            glLineWidth(15);
            glColor3f(0.8f, 0.4f, 0.4f);
            glBegin(GL_LINE_STRIP);
            for (glm::vec3 node : path) {
                glVertex3f(node.x, node.y, node.z);
            }
            glEnd();
            simpleProg->unbind();
        }
    }
}

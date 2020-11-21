#include "PathGraph.h"
//#include "PathMarker.h"
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

PathGraph::PathGraph(shared_ptr<Scene> scene_, int unitsPerNode_) :
    scene(scene_),
    unitsPerNode(unitsPerNode_),
    start(NULL),
    goal(NULL)
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
    cout << "Regenerating PathGraph." << endl;
    cout << "start:" << endl;
    printNodeData(start);
    cout << "goal:" << endl;
    printNodeData(goal);


    nodes.clear();

    float edgeLength = scene->getSize();
    int n = ceil(edgeLength / unitsPerNode);
    float dx = edgeLength / n;

    float pos0 = -edgeLength / 2 + dx / 2;
    for (int i = 0; i < n; i++) {
        vector < shared_ptr<PathNode> > thisRow(n);

        for (int j = 0; j < n; j++) {
            // create new node
            float posX = dx * j;
            float posZ = dx * i;
            posX += randFloat(-dx / 2, dx / 2);
            posZ += randFloat(-dx / 2, dx / 2);

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

    // update start and goal links
    if (start != NULL) {
        updateStart(start->pos);
    }
    if (goal != NULL) {
        updateGoal(goal->pos);
    }

    cout << "Done regenerating PathGraph." << endl;
    cout << "start:" << endl;
    printNodeData(start);
    cout << "goal:" << endl;
    printNodeData(goal);
    cout << endl;
}

void PathGraph::regenerate(glm::vec3 start_, glm::vec3 goal_)
{
    regenerate();

    updateStart(start_);
    updateGoal(goal_);
}

void PathGraph::printNodeData(shared_ptr<PathNode> node)
{
    if (node == NULL) {
        cout << "    Node == NULL" << endl;
        return;
    }
    else {
        cout << "    Node == " << node << endl;
    }

    cout << "    Node has " << node->neighbors.size() << " neighbors." << endl;
    for (auto neighbor : node->neighbors) {
        cout << "    - " << neighbor;
        if (neighbor == start) {
            cout << " (START)";
        }
        else if (neighbor == goal) {
            cout << " (GOAL)";
        }
        cout << endl;
    }
}

void PathGraph::updateStart(glm::vec3 pos)
{
    cout << "Updating start." << endl;
    cout << "start:" << endl;
    printNodeData(start);
    cout << "goal:" << endl;
    printNodeData(goal);

    // update old start
    if (start != NULL) {
        start->clearNeighbors();
        start->pos = pos;
    }
    else {
        start = make_shared<PathNode>(pos);
    }

    // link to new neighbors
    float edgeLength = scene->getSize();
    int n = ceil(edgeLength / unitsPerNode);
    float dx = edgeLength / n;

    int row = floor((pos.z + edgeLength / 2) / dx);
    int col = floor((pos.x + edgeLength / 2) / dx);

    shared_ptr<PathNode> mainNode = nodes[row][col];
    for (auto node : mainNode->neighbors) {
        start->addNeighbor(node);
    }
    start->addNeighbor(mainNode);

    cout << "Done updating start." << endl;
    cout << "start:" << endl;
    printNodeData(start);
    cout << "goal:" << endl;
    printNodeData(goal);
    cout << endl;
}

void PathGraph::updateGoal(glm::vec3 pos)
{
    cout << "Updating goal." << endl;
    cout << "start:" << endl;
    printNodeData(start);
    cout << "goal:" << endl;
    printNodeData(goal);


    // update old start
    if (goal != NULL) {
        goal->clearNeighbors();
        goal->pos = pos;
    }
    else {
        goal = make_shared<PathNode>(pos);
    }

    // link to new neighbors
    float edgeLength = scene->getSize();
    int n = ceil(edgeLength / unitsPerNode);
    float dx = edgeLength / n;

    int row = floor((pos.z + edgeLength / 2) / dx);
    int col = floor((pos.x + edgeLength / 2) / dx);

    shared_ptr<PathNode> mainNode = nodes[row][col];
    for (auto node : mainNode->neighbors) {
        goal->addNeighbor(node);
    }
    goal->addNeighbor(mainNode);

    cout << "Done updating goal." << endl;
    cout << "start:" << endl;
    printNodeData(start);
    cout << "goal:" << endl;
    printNodeData(goal);
    cout << endl;
}

////////////////////////////////////////////////////////////
void PathGraph::clear35(){
    nodes[3][5]->clearNeighbors();
    nodes[3][5] = nullptr;
}
////////////////////////////////////////////////////////////

// struct for A* to track branches
struct AStarBranch
{
    AStarBranch(std::vector< std::shared_ptr<PathNode> > _path, float _g = 0) : path(_path)
    {

    }

    std::vector< std::shared_ptr<PathNode> > path;
    float g;
    float f;
};

// struct for priority queue to order elements
struct BranchCompare
{
    bool operator()(const AStarBranch &a, const AStarBranch &b)
    {
        return a.f <= b.f;
    }
};

vector< shared_ptr<PathNode> > PathGraph::findPath()
{
    // return empty path if start or goal is not available
    if (start == NULL || goal == NULL) {
        return vector< shared_ptr<PathNode> >();
    }

    // do A* search from start to goal
    vector< shared_ptr<PathNode> > startPath;
    startPath.push_back(start);
    AStarBranch startBranch(startPath);
    priority_queue<AStarBranch, vector<AStarBranch>, BranchCompare> pq;
    pq.push(startBranch);

    shared_ptr<AStarBranch> currentBranch = make_shared<AStarBranch>(pq.top());
    pq.pop();
    while (currentBranch->path.back() != goal && !pq.empty()) {
        // expand current branch
        for (auto node : currentBranch->path.back()->neighbors) {

        }

        // go to next branch in pq
        currentBranch = make_shared<AStarBranch>(pq.top());
        pq.pop();
    }

    if (currentBranch->path.back() != goal) {
        // no path from start to goal exists
        cout << "PathGraph::findPath(): No path from start to goal exists." << endl;
    }

    return currentBranch->path;
}

void PathGraph::draw(shared_ptr<MatrixStack> P, shared_ptr<MatrixStack> MV, vector< shared_ptr<PathNode> > path)
{
    // --- draw normal nodes ---
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
            if (node != NULL) {
                MV->pushMatrix();

                MV->translate(node->pos);
                MV->translate(glm::vec3(-0.75f, 0.0f, -0.75f));
                glUniformMatrix4fv(shapeProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
                PmShape->draw();

                MV->popMatrix();
            }
        }
    }

    shapeProg->unbind();

    // --- draw each edge ---
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
            if (node != NULL) {
                glm::vec3 pos0 = node->pos;
                for (auto neighbor : node->neighbors) {
                    glm::vec3 pos1 = neighbor->pos;

                    glVertex3f(pos0.x, pos0.y, pos0.z);
                    glVertex3f(pos1.x, pos1.y, pos1.z);
                }
            }
        }
    }

    glEnd();
    simpleProg->unbind();

    // --- draw start & goal ---
    // start
    if (start != NULL) {
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

        // draw links
        simpleProg->bind();
        glLineWidth(8);
        glColor3f(0.9f, 0.9f, 0.0f);
        glBegin(GL_LINES);
        glm::vec3 pos0 = start->pos;
        for (auto neighbor : start->neighbors) {
            glm::vec3 pos1 = neighbor->pos;

            glVertex3f(pos0.x, pos0.y, pos0.z);
            glVertex3f(pos1.x, pos1.y, pos1.z);
        }
        glEnd();
        simpleProg->unbind();
    }
    
    // goal
    if (goal != NULL) {
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

        // draw links
        simpleProg->bind();
        glLineWidth(8);
        glColor3f(0.0f, 0.9f, 0.0f);
        glBegin(GL_LINES);
        glm::vec3 pos0 = goal->pos;
        for (auto neighbor : goal->neighbors) {
            glm::vec3 pos1 = neighbor->pos;

            glVertex3f(pos0.x, pos0.y, pos0.z);
            glVertex3f(pos1.x, pos1.y, pos1.z);
        }
        glEnd();
        simpleProg->unbind();
    }
    
    // --- draw selected path ---
    if (path.size() > 0) {
        simpleProg->bind();
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
        simpleProg->unbind();
    }

}

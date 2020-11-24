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

PathGraph::PathGraph(float _edgeLength, int _unitsPerNode) :
    edgeLength(_edgeLength),
    unitsPerNode(_unitsPerNode),
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
    //cout << "Regenerating PathGraph." << endl;
    //cout << "start:" << endl;
    //printNodeData(start);
    //cout << "goal:" << endl;
    //printNodeData(goal);

    // remove start & goal from rest of graph
    if (start != NULL) {
        start->clearNeighbors();
    }
    if (goal != NULL) {
        goal->clearNeighbors();
    }

    nodes.clear();

    //float edgeLength = scene->getSize();
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

    //cout << "Done regenerating PathGraph." << endl;
    //cout << "start:" << endl;
    //printNodeData(start);
    //cout << "goal:" << endl;
    //printNodeData(goal);
    //cout << endl;
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
    //cout << "Updating start." << endl;
    //cout << "start:" << endl;
    //printNodeData(start);
    //cout << "goal:" << endl;
    //printNodeData(goal);

    // update old start
    if (start != NULL) {
        if (start->pos == pos) {
            // don't need to update start
            return;
        }

        start->clearNeighbors();
        start->pos = pos;
    }
    else {
        start = make_shared<PathNode>(pos);
    }

    // link to new neighbors
    //float edgeLength = scene->getSize();
    int n = ceil(edgeLength / unitsPerNode);
    float dx = edgeLength / n;

    int row = floor((pos.z + edgeLength / 2) / dx);
    int col = floor((pos.x + edgeLength / 2) / dx);

    shared_ptr<PathNode> mainNode = nodes[row][col];
    for (auto node : mainNode->neighbors) {
        start->addNeighbor(node);
    }
    start->addNeighbor(mainNode);

    //cout << "Done updating start." << endl;
    //cout << "start:" << endl;
    //printNodeData(start);
    //cout << "goal:" << endl;
    //printNodeData(goal);
    //cout << endl;
}

void PathGraph::updateGoal(glm::vec3 pos)
{
    //cout << "Updating goal." << endl;
    //cout << "start:" << endl;
    //printNodeData(start);
    //cout << "goal:" << endl;
    //printNodeData(goal);

    // update old goal
    if (goal != NULL) {
        if (goal->pos == pos) {
            // don't need to update goal
            return;
        }

        goal->clearNeighbors();
        goal->pos = pos;
    }
    else {
        goal = make_shared<PathNode>(pos);
    }

    // link to new neighbors
    //float edgeLength = scene->getSize();
    int n = ceil(edgeLength / unitsPerNode);
    float dx = edgeLength / n;

    int row = floor((pos.z + edgeLength / 2) / dx);
    int col = floor((pos.x + edgeLength / 2) / dx);

    shared_ptr<PathNode> mainNode = nodes[row][col];
    for (auto node : mainNode->neighbors) {
        goal->addNeighbor(node);
    }
    goal->addNeighbor(mainNode);

    //cout << "Done updating goal." << endl;
    //cout << "start:" << endl;
    //printNodeData(start);
    //cout << "goal:" << endl;
    //printNodeData(goal);
    //cout << endl;
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
    AStarBranch(shared_ptr<PathNode> start) : g(0), f(0)
    {
        path.push_back(start);
    }

    AStarBranch(shared_ptr<AStarBranch> oldBranch, shared_ptr<PathNode> newNode, float h)
    {
        // set up path
        path = oldBranch->path;
        path.push_back(newNode);

        // calculate g & f
        glm::vec3 pos0 = oldBranch->path.back()->pos;
        glm::vec3 pos1 = newNode->pos;
        glm::vec3 dx(pos1 - pos0);

        g = oldBranch->g + sqrt(dx.x * dx.x + dx.z * dx.z);
        //g = oldBranch->g + (dx.x * dx.x + dx.z * dx.z);//////////////////////////////////////////////////////////////////////////////////////////////////
        f = g + h;
    }

    std::vector< std::shared_ptr<PathNode> > path;

    // g(n) = (total distance traveled)^2
    // h(n) = (direct distance to goal)^2
    // f(n) = g(n) + h(n)
    float g;
    float f;
};

// struct for priority queue to order elements
struct BranchCompare
{
    bool operator()(const shared_ptr<AStarBranch> &a, const shared_ptr<AStarBranch> &b)
    {
        return a->f > b->f;
    }
};

void PathGraph::printBranchData(shared_ptr<AStarBranch> b, string gap)
{
    string gap2 = "    ";
    string str = "  - ";
    cout << gap << "  Printing data for branch " << b << endl;

    cout << gap << str << "g = " << b->g << endl;
    cout << gap << str << "f = " << b->f << endl;
    cout << gap << str << "path: " << endl;
    for (auto node : b->path) {
        //float edgeLength = scene->getSize();
        int n = ceil(edgeLength / unitsPerNode);
        float dx = edgeLength / n;

        int row = floor((node->pos.z + edgeLength / 2) / dx);
        int col = floor((node->pos.x + edgeLength / 2) / dx);

        cout << gap << gap2 << str << "(" << row << ", " << col << ")";
        if (node == start) {
            cout << " [start]";
        }
        if (node == goal) {
            cout << " [goal]";
        }
        cout << endl;
    }
}

vector< glm::vec3 > PathGraph::findPath()
{
    bool doOut = false;
    string str = "    ";
    if (doOut)cout << "------------------------------------------------" << endl;
    if (doOut)cout << str << "Starting findPath()." << endl;

    // return empty path if start or goal is not available
    if (start == NULL || goal == NULL) {
        if (doOut)cout << str << "Either start or goal is unavailable; exiting." << endl;
        if (doOut)cout << "------------------------------------------------" << endl << endl;
        return vector<glm::vec3>();
    }

    // do A* search from start to goal
    priority_queue<shared_ptr<AStarBranch>, vector< shared_ptr<AStarBranch> >, BranchCompare> pq;
    shared_ptr<AStarBranch> currentBranch = make_shared<AStarBranch>(start);
    if (doOut)cout << str << "currentBranch at the beginning:" << endl;
    if (doOut)printBranchData(currentBranch);
    while (currentBranch->path.back() != goal) {
        if (doOut)cout << str << "Beginning while loop." << endl;

        // expand current branch
        for (auto node : currentBranch->path.back()->neighbors) {
            glm::vec3 dx(goal->pos - node->pos);
            float h = sqrt(dx.x * dx.x + dx.z * dx.z);///////////////////////////////////////////////////////////////////temp don't need to use sqrt
            //float h = dx.x * dx.x + dx.z * dx.z;
            shared_ptr<AStarBranch> newBranch = make_shared<AStarBranch>(currentBranch, node, h);
            pq.push(newBranch);
            if (doOut)cout << str << str << "goal->pos = (" << goal->pos.x << ", " << goal->pos.z << "), node->pos = (" << node->pos.x << ", " << node->pos.z << "), h = " << h << endl;
            //if (doOut)cout << str << str << "dx = (" << dx.x << ", " << dx.z << "), node->pos = (" << node->pos.x << ", " << node->pos.z << "), h = " << h << endl;
            if (doOut)cout << str << str << "Adding branch to pq." << endl;
            if (doOut)printBranchData(newBranch, string("        "));
        }

        // go to next branch in pq
        currentBranch = pq.top();
        if (doOut)cout << str << "selected next branch to follow:" << endl;
        if (doOut)printBranchData(currentBranch);
        pq.pop();
        if (doOut)cout << str << "^^^ Popped that branch from pq." << endl;
        if (pq.empty()) {
            if (doOut)cout << str << "!!! pq is empty; exiting while loop." << endl;
            break;
        }
        if (doOut)cout << endl;
    }

    if (currentBranch->path.back() != goal) {
        // no path from start to goal exists
        cout << "PathGraph::findPath(): No path from start to goal exists." << endl;
        if (doOut)cout << str << endl << "Exiting findPath()." << endl;
        if (doOut)cout << "------------------------------------------------" << endl;
        return vector<glm::vec3>();
    }

    if (doOut)cout << str << " !!!! Optimal branch selected !!!!" << endl;
    if (doOut)printBranchData(currentBranch);
    if (doOut)cout << str << endl << "Exiting findPath()." << endl;
    if (doOut)cout << "------------------------------------------------" << endl << endl << endl;

    // convert branch path to vector of glm::vec3
    vector<glm::vec3> finalPath;
    for (auto node : currentBranch->path) {
        finalPath.push_back(node->pos);
    }

    return finalPath;
}

void PathGraph::draw(shared_ptr<MatrixStack> P, shared_ptr<MatrixStack> MV, vector<glm::vec3> &path)
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
                ////////////////////////////////////////////////////////////////
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
                ////////////////////////////////////////////////////////////////


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

        //// draw links
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
    
    // --- draw provided path ---
    if (path.size() > 0) {
        simpleProg->bind();
        glLineWidth(15);
        glColor3f(0.8f, 0.4f, 0.4f);
        glBegin(GL_LINE_STRIP);
        //for (int i = 0; i < path.size() - 1; i++) {
        //    glm::vec3 pos0 = path[i]->pos;
        //    glm::vec3 pos1 = path[i + 1]->pos;

        //    glVertex3f(pos0.x, pos0.y, pos0.z);
        //    glVertex3f(pos1.x, pos1.y, pos1.z);
        //}
        for (glm::vec3 node : path) {
            glVertex3f(node.x, node.y, node.z);
        }
        glEnd();
        simpleProg->unbind();
    }

}

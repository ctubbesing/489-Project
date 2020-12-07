#pragma once
#ifndef PATHGRAPH_H
#define PATHGRAPH_H

#include <vector>
#include <memory>

#include <glm/glm.hpp>

class Scene;
class Shape;
class MatrixStack;
class Program;
class ProgInfo;

class PathNode : public std::enable_shared_from_this<PathNode>
{
public:
    PathNode(glm::vec3 pos_) : pos(pos_) {}
    glm::vec3 pos;
    std::vector < std::shared_ptr<PathNode> > neighbors;

    void addNeighbor(std::shared_ptr<PathNode> n)
    {
        for (auto node : neighbors) {
            if (n == node) {
                return;
            }
        }
        neighbors.push_back(n);
        n->addNeighbor(shared_from_this());
    }
    
    void removeNeighbor(std::shared_ptr<PathNode> n) {
        auto index = std::find< std::vector< std::shared_ptr<PathNode> >::iterator, std::shared_ptr<PathNode> >(neighbors.begin(), neighbors.end(), n);
        if (index != neighbors.end()) {
            neighbors.erase(index);
        }
    }

    void clearNeighbors()
    {
        // unlink all neighbors from this node
        for (auto node : neighbors) {
            node->removeNeighbor(shared_from_this());
        }
        neighbors.clear();
    }
};

// struct for A* to track branches
class AStarBranch
{
public:
    AStarBranch(std::shared_ptr<PathNode> start) : g(0), f(0)
    {
        path.push_back(start);
    }

    AStarBranch(std::shared_ptr<AStarBranch> oldBranch, std::shared_ptr<PathNode> newNode, float h)
    {
        // set up path
        path = oldBranch->path;
        path.push_back(newNode);

        // calculate g & f
        glm::vec3 pos0 = oldBranch->path.back()->pos;
        glm::vec3 pos1 = newNode->pos;
        glm::vec3 dx(pos1 - pos0);

        g = oldBranch->g + sqrt(dx.x * dx.x + dx.z * dx.z);
        f = g + h;
    }

    std::vector< std::shared_ptr<PathNode> > path;
    float g;
    float f;
};

class PathGraph
{
public:
    PathGraph();
    PathGraph(const std::shared_ptr<Scene> _scene, ProgInfo progs, std::string DATA_DIR = "", int _unitsPerNode = 20);
    PathGraph(const PathGraph &pg);
    virtual ~PathGraph();

    void regenerate();
    void regenerate(glm::vec3 start, glm::vec3 goal);
    void updateStart(glm::vec3 pos);
    void updateGoal(glm::vec3 pos);

    std::vector< glm::vec3 > findPath();

    void draw(
        std::shared_ptr<MatrixStack> P,
        std::shared_ptr<MatrixStack> MV,
        std::vector<glm::vec3> &path = std::vector<glm::vec3>(),
        bool isSelected = false,
        bool drawFullPG = true,
        bool drawPath = true
    );

private:
    std::vector< std::vector< std::shared_ptr<PathNode> > > nodes;
    std::shared_ptr<PathNode> start;
    std::shared_ptr<PathNode> goal;

    std::shared_ptr<Shape> PmShape;
    std::shared_ptr<Scene> scene;
    float edgeLength;
    int unitsPerNode;

    std::shared_ptr<Program> simpleProg;
    std::shared_ptr<Program> shapeProg;

    float randFloat(float l, float h);
    bool isClearPath(std::shared_ptr<PathNode> a, std::shared_ptr<PathNode> b);
};

#endif
#pragma once

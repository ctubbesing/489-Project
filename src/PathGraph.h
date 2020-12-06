#pragma once
#ifndef PATHGRAPH_H
#define PATHGRAPH_H

#include <vector>
#include <memory>

#include <glm/glm.hpp>

//#include "Shape.h"

//#include "PathMarker.h"
//#include "Scene.h"
//#include "Shape.h"
//#include "MatrixStack.h"

class Scene;
class Shape;
class MatrixStack;
class Program;

////////////////////////////////////////////////
struct AStarBranch;
////////////////////////////////////////////////

class PathNode : std::enable_shared_from_this<PathNode>
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

class PathGraph
{
public:
    PathGraph();
    PathGraph(const std::shared_ptr<Scene> _scene, float _edgeLength = 100.0f, int _unitsPerNode = 10);
    virtual ~PathGraph();
    void regenerate();
    void regenerate(glm::vec3 start, glm::vec3 goal);
    void updateStart(glm::vec3 pos);
    void updateGoal(glm::vec3 pos);
    std::vector< glm::vec3 > findPath();
    void setSimpleProgram(std::shared_ptr<Program> p) { simpleProg = p; }
    void setShapeProgram(std::shared_ptr<Program> p) { shapeProg = p; }
    void setShape(std::shared_ptr<Shape> shape) { PmShape = shape; }

    void draw(std::shared_ptr<MatrixStack> P, std::shared_ptr<MatrixStack> MV, std::vector<glm::vec3> &path = std::vector<glm::vec3>(), bool drawFullPG = true, bool drawPath = true);

    ///////////////////////////////////////////////////
    void clear35();
    void printNodeData(std::shared_ptr<PathNode> node);
    void printBranchData(std::shared_ptr<AStarBranch> b, std::string gap = std::string("    "));
    ///////////////////////////////////////////////////
private:
    std::vector< std::vector< std::shared_ptr<PathNode> > > nodes;
    std::shared_ptr<PathNode> start;
    std::shared_ptr<PathNode> goal;
    //std::vector< std::shared_ptr<PathNode> > nodes;
    std::shared_ptr<Scene> scene;
    float edgeLength;
    int unitsPerNode;
    std::shared_ptr<Program> simpleProg;
    std::shared_ptr<Program> shapeProg;
    std::shared_ptr<Shape> PmShape;

    float randFloat(float l, float h);
    bool isClearPath(std::shared_ptr<PathNode> a, std::shared_ptr<PathNode> b);
};

#endif
#pragma once

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

struct PathNode
{
    PathNode(glm::vec3 pos_) : pos(pos_) {}
    glm::vec3 pos;
    std::vector < std::shared_ptr<PathNode> > neighbors;
};

class PathGraph
{
public:
    //PathGraph();
    PathGraph(std::shared_ptr<Scene> scene_);
    virtual ~PathGraph();
    void regenerate();
    std::vector< std::shared_ptr<PathNode> > findPath(std::shared_ptr<PathNode> start, std::shared_ptr<PathNode> goal);
    void setSimpleProgram(std::shared_ptr<Program> p) { simpleProg = p; }
    void setShapeProgram(std::shared_ptr<Program> p) { shapeProg = p; }
    void setShape(std::shared_ptr<Shape> shape) { PmShape = shape; }

    void draw(std::shared_ptr<MatrixStack> P, std::shared_ptr<MatrixStack> MV, std::vector< std::shared_ptr<PathNode> > path = std::vector< std::shared_ptr<PathNode> >());

private:
    std::vector< std::shared_ptr<PathNode> > nodes;
    std::shared_ptr<Scene> scene;
    std::shared_ptr<Program> simpleProg;
    std::shared_ptr<Program> shapeProg;
    std::shared_ptr<Shape> PmShape;

    float randFloat(float l, float h);
};

#endif
#pragma once

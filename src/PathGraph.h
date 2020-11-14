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
    PathGraph();
    virtual ~PathGraph();
    void regenerate(std::shared_ptr<Scene> scene);
    std::vector< std::shared_ptr<PathNode> > findPath(std::shared_ptr<PathNode> start, std::shared_ptr<PathNode> goal);
    void draw(std::shared_ptr<MatrixStack> MV, std::vector< std::shared_ptr<PathNode> > path = std::vector< std::shared_ptr<PathNode> >());

private:
    std::vector< std::shared_ptr<PathNode> > nodes;
    std::shared_ptr<Scene> scene;
    std::shared_ptr<Program> simpleProg;
    std::shared_ptr<Program> shapeProg;
    std::shared_ptr<Shape> PmShape;
};

#endif
#pragma once

#pragma once
#ifndef PATHMARKER_H
#define PATHMARKER_H

#include <vector>
#include <memory>

#include <glm/glm.hpp>
//#include "Shape.h"
//#include "MatrixStack.h"

class Shape;
class MatrixStack;

class PathMarker
{
public:
    PathMarker();
    virtual ~PathMarker();
    //void draw(std::shared_ptr<MatrixStack> MV, std::shared_ptr<Program> prog);
    //bool checkStart() { return isStart; }
    //bool checkGoal() { return isGoal; }

private:
    //bool isStart;    // maybe just pass start & goal into PathGraph::findPath()m
    //bool isGoal;
    glm::vec3 pos;
    std::vector< std::shared_ptr<PathMarker> > neighbors;
    //Shape shape;
};

#endif
#pragma once

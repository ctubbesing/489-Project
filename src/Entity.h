#pragma once
#ifndef ENTITY_H
#define ENTITY_H

#include <vector>
#include <memory>

#include <glm/glm.hpp>
//#include "ShapeSkin.h"
//#include "MatrixStack.h"

class MatrixStack;
class ShapeSkin;
class Program;

class Entity
{
public:
    Entity();
    virtual ~Entity();
    void draw(std::shared_ptr<MatrixStack> MV, std::shared_ptr<Program> prog);

private:
    glm::vec3 pos;
    //Shape shape;
};

#endif
#pragma once

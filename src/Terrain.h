#pragma once
#ifndef TERRAIN_H
#define TERRAIN_H

#include <vector>
#include <memory>

#include <glm/glm.hpp>

#include "Shape.h"
//#include "MatrixStack.h"

class MatrixStack;

class Terrain : public Shape
{
public:
    Terrain();
    Terrain(float d, int n, bool flat = false);
    virtual ~Terrain();
    void init();
    void generateTerrain(bool flat = false);
    float getAltitude(float x, float z);
    glm::vec3 getPoint(int x, int z = -1);
    float getSize() { return edgeLength; }

    void draw(std::shared_ptr<MatrixStack> MV, const std::shared_ptr<Program> p);

private:
    std::vector< std::vector<glm::vec3> > landMat;
    float edgeLength;
    int edgeCells;

    std::vector<unsigned int> eleBuf;
    unsigned eleBufID;

    float randFloat(float l, float h);
};

#endif
#pragma once

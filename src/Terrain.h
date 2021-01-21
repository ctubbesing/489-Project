#pragma once
#ifndef TERRAIN_H
#define TERRAIN_H

#include <vector>
#include <memory>

#include <glm/glm.hpp>

#include "Shape.h"

class MatrixStack;

class Terrain : public Shape
{
public:
    Terrain();
    Terrain(float _edgeLength, int _edgeCells, bool flat = false);
    virtual ~Terrain();
    void init();
    void generateTerrain(bool flat = false);
    void updatePosNor();
    float getAltitude(glm::vec3 pos);
    bool isObstacle(glm::vec3 pos);
    float getEdgeLength() { return edgeLength; }

    void draw(std::shared_ptr<MatrixStack> P, std::shared_ptr<MatrixStack> MV);

private:
    std::vector< std::vector<glm::vec3> > landMat;
    float edgeLength;
    int edgeCells;

    float randFloat(float l, float h);
    std::vector<glm::vec3> findTriangle(glm::vec3 pos, float &a, float &b);
};

#endif
#pragma once

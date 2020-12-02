#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Terrain.h"
#include "MatrixStack.h"
#include "Program.h"

#include "GLSL.h"

using namespace std;

Terrain::Terrain() :
    edgeLength(24.0f),
    edgeCells(24)
{
    int nVerts = (edgeCells + 1)*(edgeCells + 1);///////////////////////////////////////////////// just the surface for now, not the side walls
    posBuf.clear();
    norBuf.clear();
    //texBuf.clear();
    //eleBuf.clear();
    //posBuf.resize(nVerts * 3);
    //norBuf.resize(nVerts * 3);

    generateTerrain(true);

    init();
}

Terrain::Terrain(float d, int n, bool flat) :
    edgeLength(d),
    edgeCells(n)
{
    int nVerts = (edgeCells + 1)*(edgeCells + 1);///////////////////////////////////////////////// just the surface for now, not the side walls
    posBuf.clear();
    norBuf.clear();
    //texBuf.clear();
    //eleBuf.clear();
    //posBuf.resize(nVerts * 3);
    //norBuf.resize(nVerts * 3);

    generateTerrain(flat);

    init();
}

void Terrain::init()
{
    glGenBuffers(1, &posBufID);
    glBindBuffer(GL_ARRAY_BUFFER, posBufID);
    glBufferData(GL_ARRAY_BUFFER, posBuf.size() * sizeof(float), &posBuf[0], GL_DYNAMIC_DRAW);

    glGenBuffers(1, &norBufID);
    glBindBuffer(GL_ARRAY_BUFFER, norBufID);
    glBufferData(GL_ARRAY_BUFFER, norBuf.size() * sizeof(float), &norBuf[0], GL_DYNAMIC_DRAW);

    //glGenBuffers(1, &texBufID);
    //glBindBuffer(GL_ARRAY_BUFFER, texBufID);
    //glBufferData(GL_ARRAY_BUFFER, texBuf.size() * sizeof(float), &texBuf[0], GL_STATIC_DRAW);

    //glGenBuffers(1, &eleBufID);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, eleBuf.size() * sizeof(unsigned int), &eleBuf[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    assert(glGetError() == GL_NO_ERROR);
}

Terrain::~Terrain()
{

}

float Terrain::randFloat(float l, float h)
{
    float r = rand() / (float)RAND_MAX;
    return (1.0f - r) * l + r * h;
}

vector<glm::vec3> Terrain::findTriangle(glm::vec3 pos, float &a, float &b)
{
    int rows = edgeCells + 1;
    int cols = rows;

    // find row/col of landMat where point lies
    float dx = edgeLength / edgeCells;
    int row0 = floor((pos.z + edgeLength / 2) / dx);
    int col0 = floor((pos.x + edgeLength / 2) / dx);

    cout << "row0 = " << row0 << endl;
    cout << "col0 = " << col0 << endl;

    // test the surrounding triangles using barycentric coordinates
    int i_start = (row0 == 0 ? 0 : -1);
    int j_start = (col0 == 0 ? 0 : -1);
    int i_end = (row0 == rows - 1 ? 0 : 1);
    int j_end = (col0 == cols - 1 ? 0 : 1);

    for (int i = i_start; i < i_end; i++) {
        for (int j = j_start; j < j_end; j++) {
            glm::vec3 _v0 = landMat[row0 + i][col0 + j];
            glm::vec3 _v1 = landMat[row0 + i + 1][col0 + j];
            glm::vec3 _v2 = landMat[row0 + i][col0 + j + 1];
            glm::vec3 _v3 = landMat[row0 + i + 1][col0 + j + 1];
            vector<glm::vec3> v0123 = { _v0, _v1, _v2, _v3 };

            for (int k = 0; k < 2; k++) {
                glm::vec3 v0 = v0123[k];
                glm::vec3 v1 = v0123[k + 1];
                glm::vec3 v2 = v0123[k + 2];
                float denom = (v1.z - v2.z)*(v0.x - v2.x) + (v2.x - v1.x)*(v0.z - v2.z);
                a = ((v1.z - v2.z)*(pos.x - v2.x) + (v2.x - v1.x)*(pos.z - v2.z)) / denom;
                b = ((v2.z - v0.z)*(pos.x - v2.x) + (v0.x - v2.x)*(pos.z - v2.z)) / denom;
                float c = 1 - a - b;

                if (a >= 0 && a <= 1 && b >= 0 && b <= 1 && c >= 0 && c <= 1) {
                    // pos is in this triangle
                    vector<glm::vec3> vec = { v0, v1, v2 };
                    return vec;
                }
            }
        }
    }
    cout << "not supposed to be here :( no triangle found" << endl;
}

void Terrain::generateTerrain(bool flat)
{
    landMat.clear();
    //obstacles.clear();

    int rows = edgeCells + 1;
    int cols = rows;

    // generate flat x, z grid
    landMat = vector< vector<glm::vec3> >(rows, vector<glm::vec3>(cols, glm::vec3(-edgeLength / 2, 0.0f, -edgeLength / 2)));
    float dx = edgeLength / edgeCells;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            landMat[i][j].z += i * dx;
            landMat[i][j].x += j * dx;

            /////////////////////////////////// temp
            if (i < rows / 2) landMat[i][j].y += 8.0f;
            /////////////////////////////////// temp
        }
    }

    // generate random terrain
    if (!flat) {
        // randomize x & z

        //rows = 15;////////////////////////////////////////////////////////////////////////////////////////////////

        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                if (i != 0 && i != rows - 1) {
                    landMat[i][j].z += randFloat(-dx / 2, dx / 2);
                }
                if (j != 0 && j != cols - 1) {
                    // make sure vertex isn't inside neighboring triangle /////////////////////////////////////////// not yet implemented
                    float l = -dx / 2;
                    //if (i > 0) {
                    //    float c = (landMat[i][j].z - landMat[i][j - 1].z) / (landMat[i - 1][j].z - landMat[i][j - 1].z);
                    //    float xmin = c * (landMat[i - 1][j].x - landMat[i][j - 1].x) + landMat[i][j - 1].x;
                    //    l = ((landMat[i][j].x - dx / 2) >= xmin ? (-dx / 2) : (landMat[i][j].x - xmin));
                    //}

                    landMat[i][j].x += randFloat(l, dx / 2);
                }

                /////////////////////////////////// temp
                //landMat[i][j].y = randFloat(0.0f, 0.4f);
                if (i < rows / 4) landMat[i][j].y += 16.0f;
                int iMid = i - edgeLength / 2;
                int jMid = j - edgeLength / 2;
                float dist = 0.5 * sqrt(iMid * iMid + jMid * jMid) - 8.5f;
                if (j < cols / 3) landMat[i][j].y += dist;
                /////////////////////////////////// temp
            }
        }

        // do y values...

    }

    updatePosNor();
}

void Terrain::updatePosNor()
{
    int rows = edgeCells + 1;
    int cols = rows;

    // update vertex buffers
    /*
    // position /////////////////////////////// maybe can just do posbuf = memcpy(landMat) ? dont need to tho
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            int k = i * cols + j;
            glm::vec3 x = landMat[i][j];
            posBuf[3 * k + 0] = x[0];
            posBuf[3 * k + 1] = x[1];
            posBuf[3 * k + 2] = x[2];
        }
    }

    // normal
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            // Each vertex has six neighbors
            //
            //      v1--d1
            //     /|  /|
			//    / | / |
            //  u0--XX--u1
            //  |  /|  /
            //  | / | /
            //  d0--v0
            //
            // Use these six triangles to compute the normal
            int k = i * cols + j;
            glm::vec3 x = landMat[i][j];
            glm::vec3 xu0, xu1, xv0, xv1, xd0, xd1, c;
            glm::vec3 nor(0.0, 0.0, 0.0);
            //int count = 0;
            // Top-right lower triangle
            if (i != 0 && j != cols - 1) {
                xu1 = landMat[i][j + 1];
                xd1 = landMat[i - 1][j + 1];
                c = cross(xu1 - x, xd1 - x);
                nor += normalize(c);
                //++count;
            }
            // Top-right upper triangle
            if (i != 0 && j != cols - 1) {
                xd1 = landMat[i - 1][j + 1];
                xv1 = landMat[i - 1][j];
                c = cross(xd1 - x, xv1 - x);
                nor += normalize(c);
                //++count;
            }
            // Top-left triangle
            if (i != 0 && j != 0) {
                xv1 = landMat[i - 1][j];
                xu0 = landMat[i][j - 1];
                c = cross(xv1 - x, xu0 - x);
                nor += normalize(c);
                //++count;
            }
            // Bottom-left upper triangle
            if (i != rows - 1 && j != 0) {
                xu0 = landMat[i][j - 1];
                xd0 = landMat[i + 1][j - 1];
                c = cross(xu0 - x, xd0 - x);
                nor += normalize(c);
                //++count;
            }
            // Bottom-left lower triangle
            if (i != rows - 1 && j != 0) {
                xd0 = landMat[i + 1][j - 1];
                xv0 = landMat[i + 1][j];
                c = cross(xd0 - x, xv0 - x);
                nor += normalize(c);
                //++count;
            }
            // Bottom-right triangle
            if (i != rows - 1 && j != cols - 1) {
                xv0 = landMat[i + 1][j];
                xu1 = landMat[i][j + 1];
                c = cross(xv0 - x, xu1 - x);
                nor += normalize(c);
                //++count;
            }
            //nor /= count;
            nor = normalize(nor);
            norBuf[3 * k + 0] = nor[0];
            norBuf[3 * k + 1] = nor[1];
            norBuf[3 * k + 2] = nor[2];
        }
    }
    //*/
    
    // position & normal
    // add triangles to buffers 2 at a time in this order:
    // 0   2 5
    // | / / |
    // |/ /  |
    // 1 3---4
    vector<int> i_buf = { 0, 1, 0, 1, 1, 0 };
    vector<int> j_buf = { 0, 0, 1, 0, 1, 1 };
    for (int i = 0; i < rows - 1; ++i) {
        for (int j = 0; j < cols - 1; ++j) {
            // add positions
            for (int k = 0; k < 6; k++) {
                glm::vec3 x = landMat[ (i + i_buf[k]) ][ (j + j_buf[k]) ];
                posBuf.push_back(x[0]);
                posBuf.push_back(x[1]);
                posBuf.push_back(x[2]);
            }

            // add normals
            glm::vec3 n = glm::normalize(glm::cross(landMat[i + 1][j] - landMat[i][j], landMat[i][j + 1] - landMat[i][j]));
            for (int k = 0; k < 3; k++) {
                norBuf.push_back(n[0]);
                norBuf.push_back(n[1]);
                norBuf.push_back(n[2]);
            }
            n = glm::normalize(glm::cross(landMat[i + 1][j + 1] - landMat[i + 1][j], landMat[i][j + 1] - landMat[i + 1][j + 1]));
            for (int k = 0; k < 3; k++) {
                norBuf.push_back(n[0]);
                norBuf.push_back(n[1]);
                norBuf.push_back(n[2]);
            }
        }
    }

    // normal
    //// elements
    //for (int i = 0; i < posBuf.size(); i++) {
    //    eleBuf.push_back(i);
    //}
    //for (int i = 0; i < rows - 1; ++i) {
    //    for (int j = 0; j < cols - 1; ++j) {
    //        int k0 = i * cols + j;
    //        int k1 = k0 + cols;
    //        int k2 = k0 + 1;
    //        int k4 = k1 + 1;
    //        // individual triangles (not triangle strip)
    //        eleBuf.push_back(k0);
    //        eleBuf.push_back(k1);
    //        eleBuf.push_back(k2);
    //        eleBuf.push_back(k1);
    //        eleBuf.push_back(k4);
    //        eleBuf.push_back(k2);
    //    }
    //}
}

float Terrain::getAltitude(glm::vec3 pos)
{
    float a, b, c;
    vector<glm::vec3> v = findTriangle(pos, a, b);

    c = 1 - a - b;

    float y = a * v[0].y + b * v[1].y + c * v[2].y;

    return y;
}

glm::vec3 Terrain::getPoint(int i, int j)
{
    int rows = edgeCells + 1;
    if (j == -1) {
        return landMat[i % rows][i / rows];
    }
    else {
        return landMat[i][j];
    }
}

void Terrain::draw(shared_ptr<MatrixStack> P, shared_ptr<MatrixStack> MV)
{
    int rows = edgeCells + 1;
    int cols = rows;

    // Draw mesh
    glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));

    glm::vec3 grass(0.376f, 0.502f, 0.22f);
    float diffAmt = 0.4f;
    glUniform3f(prog->getUniform("ka"), grass.x, grass.y, grass.z);
    glUniform3f(prog->getUniform("kd"), grass.x * diffAmt, grass.y * diffAmt, grass.z * diffAmt);

    MV->pushMatrix();

    glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));

    int h_pos = prog->getAttribute("aPos");
    glEnableVertexAttribArray(h_pos);
    glBindBuffer(GL_ARRAY_BUFFER, posBufID);
    glBufferData(GL_ARRAY_BUFFER, posBuf.size() * sizeof(float), &posBuf[0], GL_DYNAMIC_DRAW);
    glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

    int h_nor = prog->getAttribute("aNor");
    glEnableVertexAttribArray(h_nor);
    glBindBuffer(GL_ARRAY_BUFFER, norBufID);
    glBufferData(GL_ARRAY_BUFFER, norBuf.size() * sizeof(float), &norBuf[0], GL_DYNAMIC_DRAW);
    glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

    int count = posBuf.size() / 3; // number of indices to be rendered
    glDrawArrays(GL_TRIANGLES, 0, count);

    glDisableVertexAttribArray(h_nor);
    glDisableVertexAttribArray(h_pos);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    MV->popMatrix();

    GLSL::checkError(GET_FILE_LINE);
}

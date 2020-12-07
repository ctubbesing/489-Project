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
    int nVerts = (edgeCells + 1)*(edgeCells + 1);
    posBuf.clear();
    norBuf.clear();

    generateTerrain(true);

    init();
}

Terrain::Terrain(float _edgeLength, int _edgeCells, bool flat) :
    edgeLength(_edgeLength),
    edgeCells(_edgeCells)
{
    int nVerts = (edgeCells + 1)*(edgeCells + 1);
    posBuf.clear();
    norBuf.clear();

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
    int row0 = floor((pos.z + (edgeLength + dx) / 2) / dx);
    int col0 = floor((pos.x + (edgeLength + dx) / 2) / dx);

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

    // should never reach this spot
    cout << "not supposed to be here :( no triangle found" << endl;
}

void Terrain::generateTerrain(bool flat)
{
    landMat.clear();

    int rows = edgeCells + 1;
    int cols = rows;

    // generate flat x, z grid
    landMat = vector< vector<glm::vec3> >(rows, vector<glm::vec3>(cols, glm::vec3(-edgeLength / 2, 0.0f, -edgeLength / 2)));
    float dx = edgeLength / edgeCells;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            landMat[i][j].z += i * dx;
            landMat[i][j].x += j * dx;
        }
    }

    // generate random terrain
    if (!flat) {
        // randomize x & z
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                if (i != 0 && i != rows - 1) {
                    landMat[i][j].z += randFloat(-dx / 2, dx / 2);
                }
                if (j != 0 && j != cols - 1) {
                    landMat[i][j].x += randFloat(-dx / 2, dx / 2);
                }

                // define terrain altitude
                float iPct = (float)i / rows;
                float jPct = (float)j / rows;
                float r1l = 0.4f;
                float r1h = 0.8f;
                if (iPct <= 0.2f && jPct <= 0.4f ||
                    iPct <= 0.8f && iPct > 0.2f && jPct <= 0.6f && jPct > 0.2f ||
                    iPct <= 0.8f && iPct > 0.5f && jPct <= 0.8f && jPct > 0.2f
                    ) {
                    landMat[i][j].y = 10;
                }
                else if (
                    iPct <= 0.4f && jPct > 0.8f
                    ) {
                    landMat[i][j].y = 20;
                }
                else if (iPct <= 0.2f) {
                    float a = 10.0f / (r1h - r1l);
                    float b = 20.0f - r1h * a;
                    landMat[i][j].y = a * jPct - b;
                }
                else if (iPct <= 0.6f && jPct <= 0.2f) {
                    landMat[i][j].y = -25 * iPct + 15;
                }

                // add small random deviations for rough feel
                landMat[i][j].y += randFloat(0.0f, 0.2f);
            }
        }
    }

    updatePosNor();
}

void Terrain::updatePosNor()
{
    int rows = edgeCells + 1;
    int cols = rows;

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
}

float Terrain::getAltitude(glm::vec3 pos)
{
    float a, b, c;
    vector<glm::vec3> v = findTriangle(pos, a, b);

    c = 1 - a - b;

    float y = a * v[0].y + b * v[1].y + c * v[2].y;

    return y;
}

bool Terrain::isObstacle(glm::vec3 pos)
{
    float a, b;
    vector<glm::vec3> v = findTriangle(pos, a, b);

    glm::vec3 n = glm::normalize(glm::cross(v[1] - v[0], v[2] - v[0]));

    return (n.y * n.y < 0.2);
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

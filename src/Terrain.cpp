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
    eleBuf.clear();
    posBuf.resize(nVerts * 3);
    norBuf.resize(nVerts * 3);

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
    eleBuf.clear();
    posBuf.resize(nVerts * 3);
    norBuf.resize(nVerts * 3);

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

    glGenBuffers(1, &eleBufID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, eleBuf.size() * sizeof(unsigned int), &eleBuf[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    assert(glGetError() == GL_NO_ERROR);
}

Terrain::~Terrain()
{

}

float randFloat(float l, float h)
{
    float r = rand() / (float)RAND_MAX;
    return (1.0f - r) * l + r * h;
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

            /////////////////////////////////// temp
            //if (i < rows / 2) landMat[i][j].y += 8.0f;
            /////////////////////////////////// temp
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

                /////////////////////////////////// temp
                landMat[i][j].y = randFloat(0.0f, 0.4f);
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



    // update vertex buffers
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

    // elements
    for (int i = 0; i < rows - 1; ++i) {
        for (int j = 0; j < cols; ++j) {
            int k0 = i * cols + j;
            int k1 = k0 + cols;
            // Triangle strip
            eleBuf.push_back(k0);
            eleBuf.push_back(k1);
        }
    }
}

float Terrain::getAltitude(float x, float z)
{
    int xIndex = floor(x + edgeLength / 2); // these aren't correct jsyk
    int zIndex = floor(z + edgeLength / 2);

    float cellWidth = edgeLength / edgeCells;

    return 0.0f;
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

void Terrain::draw(shared_ptr<MatrixStack> MV, const shared_ptr<Program> p)
{
    // Draw mesh
    //glUniform3fv(p->getUniform("kdFront"), 1, glm::vec3(1.0, 0.0, 0.0).data());
    //glUniform3fv(p->getUniform("kd"), 1, glm::vec3(1.0, 1.0, 0.0));
    //glUniform3f(p->getUniform("kd"), 0.8f, 0.4f, 0.5f);
    glm::vec3 grass(0.376f, 0.502f, 0.22f);
    float diffAmt = 0.4f;
    glUniform3f(p->getUniform("ka"), grass.x, grass.y, grass.z);
    glUniform3f(p->getUniform("kd"), grass.x * diffAmt, grass.y * diffAmt, grass.z * diffAmt);


    GLSL::checkError(GET_FILE_LINE);

    MV->pushMatrix();

    GLSL::checkError(GET_FILE_LINE);

    glUniformMatrix4fv(p->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));

    GLSL::checkError(GET_FILE_LINE);

    int h_pos = p->getAttribute("aPos");
    glEnableVertexAttribArray(h_pos);

    GLSL::checkError(GET_FILE_LINE);

    glBindBuffer(GL_ARRAY_BUFFER, posBufID);

    GLSL::checkError(GET_FILE_LINE);

    glBufferData(GL_ARRAY_BUFFER, posBuf.size() * sizeof(float), &posBuf[0], GL_DYNAMIC_DRAW);
    glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
    int h_nor = p->getAttribute("aNor");
    glEnableVertexAttribArray(h_nor);
    glBindBuffer(GL_ARRAY_BUFFER, norBufID);
    glBufferData(GL_ARRAY_BUFFER, norBuf.size() * sizeof(float), &norBuf[0], GL_DYNAMIC_DRAW);
    glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID);
    int rowCols = edgeCells + 1;

    for (int i = 0; i < rowCols; ++i) {
        //float iPct = (float)i / rowCols;
        //if (i % 2 == 0) {
        //    glUniform3f(p->getUniform("ka"), iPct / 2, 0.25f, 0.4f);
        //    glUniform3f(p->getUniform("kd"), iPct, 0.5f, 0.8f);
        //}
        //else {
        //    glUniform3f(p->getUniform("ka"), 0.4, 0.25f, (1.0f - iPct)/2);
        //    glUniform3f(p->getUniform("kd"), 0.8, 0.5f, 1.0f - iPct);
        //}
        glDrawElements(GL_TRIANGLE_STRIP, 2 * rowCols, GL_UNSIGNED_INT, (const void *)(2 * rowCols*i * sizeof(unsigned int)));
    }
    glDisableVertexAttribArray(h_nor);
    glDisableVertexAttribArray(h_pos);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    MV->popMatrix();

    GLSL::checkError(GET_FILE_LINE);
}

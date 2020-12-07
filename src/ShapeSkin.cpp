#include <iostream>
#include <fstream>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "tiny_obj_loader.h"

#include "ShapeSkin.h"
#include "GLSL.h"
#include "Program.h"
#include "TextureMatrix.h"

using namespace std;
using namespace glm;

ShapeSkin::ShapeSkin() :
    prog(NULL),
    elemBufID(0),
    posBufID(0),
    norBufID(0),
    texBufID(0)
{
    T = make_shared<TextureMatrix>();
}

ShapeSkin::~ShapeSkin()
{
}

void ShapeSkin::setTextureMatrixType(const std::string &meshName)
{
    T->setType(meshName);
}

void ShapeSkin::loadMesh(const string &meshName)
{
    // Load geometry
    // This works only if the OBJ file has the same indices for v/n/t.
    // In other words, the 'f' lines must look like:
    // f 70/70/70 41/41/41 67/67/67
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    string warnStr, errStr;
    bool rc = tinyobj::LoadObj(&attrib, &shapes, &materials, &warnStr, &errStr, meshName.c_str());
    if (!rc) {
        cerr << errStr << endl;
    }
    else {
        posBuf = attrib.vertices;
        norBuf = attrib.normals;
        texBuf = attrib.texcoords;
        assert(posBuf.size() == norBuf.size());
        // Loop over shapes
        for (size_t s = 0; s < shapes.size(); s++) {
            // Loop over faces (polygons)
            const tinyobj::mesh_t &mesh = shapes[s].mesh;
            size_t index_offset = 0;
            for (size_t f = 0; f < mesh.num_face_vertices.size(); f++) {
                size_t fv = mesh.num_face_vertices[f];
                // Loop over vertices in the face.
                for (size_t v = 0; v < fv; v++) {
                    // access to vertex
                    tinyobj::index_t idx = mesh.indices[index_offset + v];
                    elemBuf.push_back(idx.vertex_index);
                }
                index_offset += fv;
                // per-face material (IGNORE)
                shapes[s].mesh.material_ids[f];
            }
        }
    }

    // save bind pose info to members that won't be modified
    for (int i = 0; i < posBuf.size(); i += 3) {
        bindPosBuf.push_back(glm::vec3(posBuf[i], posBuf[i + 1], posBuf[i + 2]));
        bindNorBuf.push_back(glm::vec3(norBuf[i], norBuf[i + 1], norBuf[i + 2]));
    }
}

void ShapeSkin::loadAttachment(const std::string &filename)
{
    boneBuf.clear();
    weightBuf.clear();

    // open file
    ifstream in;
    in.open(filename);
    if (!in.good()) {
        cout << "Cannot read " << filename << endl;
        return;
    }
    cout << "Loading " << filename << endl;

    string line;
    bool countsLoaded = false;
    int vertCount, boneCount, maxInfluences;
    while (1) {
        getline(in, line);
        if (in.eof()) {
            break;
        }
        if (line.empty()) {
            continue;
        }
        // Skip comments
        if (line.at(0) == '#') {
            continue;
        }
        // Parse lines
        stringstream ss(line);
        if (!countsLoaded) { // load vertCount, boneCount, maxInfluences
            ss >> vertCount >> boneCount >> maxInfluences;
            countsLoaded = true;
        }
        else { // load vertex data
            int influences;
            ss >> influences;
            vector<int> boneIndex(maxInfluences);
            vector<float> boneWeight(maxInfluences);

            for (int i = 0; i < influences; i++) {
                ss >> boneIndex[i] >> boneWeight[i];
            }

            boneBuf.push_back(boneIndex);
            weightBuf.push_back(boneWeight);
        }
    }
    in.close();
}

void ShapeSkin::init()
{
    // Send the position array to the GPU
    glGenBuffers(1, &posBufID);

    // Send the normal array to the GPU
    glGenBuffers(1, &norBufID);

    // Send the texcoord array to the GPU
    glGenBuffers(1, &texBufID);
    glBindBuffer(GL_ARRAY_BUFFER, texBufID);
    glBufferData(GL_ARRAY_BUFFER, texBuf.size() * sizeof(float), &texBuf[0], GL_STATIC_DRAW);

    // Send the element array to the GPU
    glGenBuffers(1, &elemBufID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elemBufID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, elemBuf.size() * sizeof(unsigned int), &elemBuf[0], GL_STATIC_DRAW);

    // Unbind the arrays
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    GLSL::checkError(GET_FILE_LINE);
}

void ShapeSkin::update(vector<glm::mat4> &bp, vector<glm::mat4> &ap)
{
    // calculate inverse bind matrices only if necessary
    if (bindMats.size() == 0) {
        for (glm::mat4 mat : bp) {
            bindMats.push_back(glm::inverse(mat));
        }
    }

    // calculate matrix products for this frame
    vector<glm::mat4> M;
    int boneCount = ap.size();
    for (int j = 0; j < boneCount; j++) {
        M.push_back(ap[j] * bindMats[j]);
    }

    // calculate new vertices/normals
    int vertCount = boneBuf.size();
    int maxInfluences = boneBuf[0].size();
    for (int i = 0; i < vertCount; i++) {
        // get w * M
        glm::mat4 wM(0.0f);
        int infl = 0;
        while (infl < maxInfluences && weightBuf[i][infl] > 0.0f) {
            wM += weightBuf[i][infl] * M[boneBuf[i][infl]];
            infl++;
        }

        // get new position
        glm::vec3 newPos = wM * glm::vec4(bindPosBuf[i], 1.0f);
        glm::vec3 newNor = wM * glm::vec4(bindNorBuf[i], 0.0f);

        // insert into posBuf/norBuf
        posBuf[3 * i] = newPos.x;
        posBuf[3 * i + 1] = newPos.y;
        posBuf[3 * i + 2] = newPos.z;
        norBuf[3 * i] = newNor.x;
        norBuf[3 * i + 1] = newNor.y;
        norBuf[3 * i + 2] = newNor.z;
    }

    // send data to shaders
    glBindBuffer(GL_ARRAY_BUFFER, posBufID);
    glBufferData(GL_ARRAY_BUFFER, posBuf.size() * sizeof(float), &posBuf[0], GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, norBufID);
    glBufferData(GL_ARRAY_BUFFER, norBuf.size() * sizeof(float), &norBuf[0], GL_DYNAMIC_DRAW);


    GLSL::checkError(GET_FILE_LINE);
}

void ShapeSkin::draw() const
{
    assert(prog);

    // Send texture matrix
    glUniformMatrix3fv(prog->getUniform("T"), 1, GL_FALSE, glm::value_ptr(T->getMatrix()));

    int h_pos = prog->getAttribute("aPos");
    glEnableVertexAttribArray(h_pos);
    glBindBuffer(GL_ARRAY_BUFFER, posBufID);
    glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

    int h_nor = prog->getAttribute("aNor");
    glEnableVertexAttribArray(h_nor);
    glBindBuffer(GL_ARRAY_BUFFER, norBufID);
    glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

    int h_tex = prog->getAttribute("aTex");
    glEnableVertexAttribArray(h_tex);
    glBindBuffer(GL_ARRAY_BUFFER, texBufID);
    glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0);

    // Draw
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elemBufID);
    glDrawElements(GL_TRIANGLES, (int)elemBuf.size(), GL_UNSIGNED_INT, (const void *)0);

    glDisableVertexAttribArray(h_nor);
    glDisableVertexAttribArray(h_pos);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    GLSL::checkError(GET_FILE_LINE);
}

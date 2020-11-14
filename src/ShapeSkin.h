#pragma once
#ifndef SHAPESKIN_H
#define SHAPESKIN_H

#include <memory>
#include <vector>

#define GLEW_STATIC
#include <GL/glew.h>

class MatrixStack;
class Program;
class TextureMatrix;

class ShapeSkin
{
public:
    ShapeSkin();
    virtual ~ShapeSkin();
    void setTextureMatrixType(const std::string &meshName);
    void loadMesh(const std::string &meshName);
    void loadAttachment(const std::string &filename);
    void setProgram(std::shared_ptr<Program> p) { prog = p; }
    void init();
    void update(int k, std::vector<glm::mat4> &bp, std::vector<glm::mat4> &ap);
    void draw(int k) const;
    void setTextureFilename(const std::string &f) { textureFilename = f; }
    std::string getTextureFilename() const { return textureFilename; }
    std::shared_ptr<TextureMatrix> getTextureMatrix() { return T; }

private:
    std::shared_ptr<Program> prog;
    std::vector<unsigned int> elemBuf;
    std::vector<float> posBuf;
    std::vector<float> norBuf;
    std::vector<float> texBuf;
    std::vector<glm::vec3> bindPosBuf;
    std::vector<glm::vec3> bindNorBuf;
    std::vector< std::vector<int> > boneBuf;
    std::vector< std::vector<float> > weightBuf;
    std::vector<glm::mat4> bindMats;

    GLuint elemBufID;
    GLuint posBufID;
    GLuint norBufID;
    GLuint texBufID;
    std::string textureFilename;
    std::shared_ptr<TextureMatrix> T;
};

#endif

//
// Created by snaki on 12/6/2020.
//

#include <GEngine.h>
#include <renderer/Renderer.h>

namespace kVox {

    VAO::VAO(const float *vertPos, int vertPosCount, const std::string &shaderToUse, Renderer &renderer)
        : renderer(renderer) {

        shaderToRenderWith = shaderToUse;
        // each vertex is 3 elements, so divide to get total vert count
        mVertCount = vertPosCount / 3;

        // generate buffer and bind for use
        glGenBuffers(1, &mVBO);
        glBindBuffer(GL_ARRAY_BUFFER, mVBO);

        // allocate buffer of specific size and copy vertex data into it
        glBufferData(GL_ARRAY_BUFFER, vertPosCount * sizeof(float), vertPos, GL_STATIC_DRAW);

        // generate and bind VAO
        glGenVertexArrays(1, &mVAO);
        glBindVertexArray(mVAO);

        // enable vert attribute 0 (position data)
        glEnableVertexAttribArray(0);
        // vert attribute 0 (position data) has 3 components per vertex (x, y, z), each is a float.
        // we don't want this data normalized (GL_FALSE), and the data is tightly packed (0).
        // there is no byte offset required (nullptr).
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    }

    VAO::~VAO() {
        if (mVBO != GL_NONE)
            glDeleteBuffers(1, &mVBO);
        if (mVAO != GL_NONE)
            glDeleteVertexArrays(1, &mVAO);
    }

    void VAO::Draw() const {
        glBindVertexArray(mVAO);
        glDrawArrays(GL_TRIANGLES, 0, mVertCount);
    }

    void VAO::SetShader(const std::string& shaderName) { shaderToRenderWith = shaderName; }

    std::string VAO::GetShader() {
        std::string _strw = shaderToRenderWith;
        return _strw;
    }

    glm::mat4 VAO::GetModelMtx() { return modelMtx; }

    void VAO::SetModelMtx(glm::mat4 modelMat) { this->modelMtx = modelMat; }

    void PrimitiveVAO::Draw() const {
        switch (primitive) {
            case CUBE: {
                CSCI441::drawSolidCube(1.0);
                break;
            }
            case CONE: {
                CSCI441::drawSolidCone(1.0,1.0,32,64);
                break;
            }
            case CYLINDER: {
                CSCI441::drawSolidCylinder(1.0,1.0,1.0,32,64);
                break;
            }
            case TORUS: {
                CSCI441::drawSolidTorus(0.5,1.0,32,32);
                break;
            }
            case SPHERE: {
                CSCI441::drawSolidSphere(1.0,32,32);
                break;
            }
        }
    }
}
//
// Created by snaki on 12/6/2020.
//

#ifndef FP_SHADER_H
#define FP_SHADER_H

#include <GL/glew.h>

namespace kVox {

    struct ShaderUniforms {         // stores the locations of all of our shader uniforms
        GLint mvpMatrix;                    // the MVP Matrix to apply
        GLint mvMatrix;
        GLint projMtx;
        GLint viewMtx;
        GLint modelMatrix;                  // model matrix
        GLint normalMtx;                    // normal matrix
        GLint eyePos;                       // camera position
        GLint materialDiffColor;            // material diffuse color
        GLint materialSpecColor;            // material specular color
        GLint materialShininess;            // material shininess factor
        GLint materialAmbColor;             // material ambient color
    };

    struct ShaderAttributes {
        GLint vPos;
        GLint vNorm;
    };

    class Shader {
    public:
        Shader(const char* vertShaderPath, const char* fragShaderPath, const char* tcsShaderPath =nullptr,
               const char* tesShaderPath =nullptr, const char* geomShaderPath =nullptr);
        ~Shader();

        bool IsGood() const;
        bool IsActive() const;
        void Activate() const;

        GLuint GetProgramHandle() const;

        ShaderUniforms uniforms;
        ShaderAttributes attributes;

    private:
        /** Handle to compiled shader program */
        GLuint mProgram = GL_NONE;

        bool mError = false;

        static GLuint LoadAndCompileShaderFromFile(const char* filePath, GLuint shaderType);
        static bool IsShaderCompiled(GLuint shader);
        static bool IsProgramLinked(GLuint program);
    };
}

#endif //FP_SHADER_H

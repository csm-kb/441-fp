//
// Created by snaki on 12/6/2020.
//

#include "renderer/Shader.h"
#include <iostream>
#include <fstream>
#include <sstream>

namespace kVox {
    Shader::Shader(const char* vertShaderPath, const char* fragShaderPath, const char* tcsShaderPath,
                   const char* tesShaderPath, const char* geomShaderPath) {
        // Compile default shader program.
        GLuint vertShader = LoadAndCompileShaderFromFile(vertShaderPath, GL_VERTEX_SHADER);
        GLuint fragShader = LoadAndCompileShaderFromFile(fragShaderPath, GL_FRAGMENT_SHADER);
        GLuint tcssShader = LoadAndCompileShaderFromFile(tcsShaderPath, GL_TESS_CONTROL_SHADER);
        GLuint tessShader = LoadAndCompileShaderFromFile(tesShaderPath, GL_TESS_EVALUATION_SHADER);
        GLuint geomShader = LoadAndCompileShaderFromFile(geomShaderPath, GL_GEOMETRY_SHADER);
        bool isTessPresent = (tcssShader != GL_NONE && tessShader != GL_NONE);
        bool isGeomPresent = (geomShader != GL_NONE);
        if(!IsShaderCompiled(vertShader) || !IsShaderCompiled(fragShader)) {
            glDeleteShader(vertShader);
            glDeleteShader(fragShader);
            mError = true;
            return;
        }
        else if (isTessPresent && (!IsShaderCompiled(tcssShader) || !IsShaderCompiled(tessShader))) {
            glDeleteShader(vertShader);
            glDeleteShader(fragShader);
            glDeleteShader(tcssShader);
            glDeleteShader(tessShader);
            mError = true;
            return;
        }
        else if (isGeomPresent && !IsShaderCompiled(geomShader)) {
            glDeleteShader(vertShader);
            glDeleteShader(fragShader);
            if (isTessPresent) {
                glDeleteShader(tcssShader);
                glDeleteShader(tessShader);
            }
            glDeleteShader(geomShader);
            mError = true;
            return;
        }

        // Assemble shader program.
        mProgram = glCreateProgram();
        glAttachShader(mProgram, vertShader);
        glAttachShader(mProgram, fragShader);
        if (isTessPresent) {
            glAttachShader(mProgram, tcssShader);
            glAttachShader(mProgram, tessShader);
        }
        if (isGeomPresent) {
            glAttachShader(mProgram, geomShader);
        }

        // Link the program.
        glLinkProgram(mProgram);

        // If linking failed, clean up and return error.
        if(!IsProgramLinked(mProgram))
        {
            glDeleteProgram(mProgram);
            glDeleteShader(vertShader);
            glDeleteShader(fragShader);
            if (isTessPresent) {
                glDeleteShader(tcssShader);
                glDeleteShader(tessShader);
            }
            if (isGeomPresent) {
                glDeleteShader(geomShader);
            }
            mError = true;
            return;
        }

        // Detach shaders after a successful link.
        glDetachShader(mProgram, vertShader);
        glDetachShader(mProgram, fragShader);
        if (isTessPresent) {
            glDetachShader(mProgram, tcssShader);
            glDetachShader(mProgram, tessShader);
        }
        if (isGeomPresent) {
            glDetachShader(mProgram, geomShader);
        }

        this->uniforms.mvpMatrix = glGetUniformLocation(this->GetProgramHandle(), "mvpMatrix");
        this->uniforms.mvMatrix = glGetUniformLocation(this->GetProgramHandle(), "mvMatrix");
        this->uniforms.viewMtx = glGetUniformLocation(this->GetProgramHandle(), "viewMtx");
        this->uniforms.projMtx = glGetUniformLocation(this->GetProgramHandle(), "projMtx");
        this->uniforms.modelMatrix = glGetUniformLocation(this->GetProgramHandle(), "modelMatrix");
        this->uniforms.normalMtx = glGetUniformLocation(this->GetProgramHandle(), "normalMtx");
        this->uniforms.eyePos = glGetUniformLocation(this->GetProgramHandle(), "eyePos");
        this->uniforms.materialDiffColor = glGetUniformLocation(this->GetProgramHandle(), "materialDiffColor");
        this->uniforms.materialSpecColor = glGetUniformLocation(this->GetProgramHandle(), "materialSpecColor");
        this->uniforms.materialShininess = glGetUniformLocation(this->GetProgramHandle(), "materialShininess");
        this->uniforms.materialAmbColor = glGetUniformLocation(this->GetProgramHandle(), "materialAmbColor");
        this->attributes.vPos  = glGetAttribLocation(this->GetProgramHandle(), "vPos");
        this->attributes.vNorm = glGetAttribLocation(this->GetProgramHandle(), "vNorm");
    }

    Shader::~Shader() {
        if (mProgram != GL_NONE) {
            glDeleteProgram(mProgram);
        }
    }

    bool Shader::IsGood() const {
        return !mError;
    }

    void Shader::Activate() const {
        glUseProgram(mProgram);
    }

    GLuint Shader::LoadAndCompileShaderFromFile(const char *filePath, GLuint shaderType) {
        if (filePath == nullptr) {
            return GL_NONE;
        }
        std::ifstream file(filePath, std::ios::in);
        if(!file.good()) {
            std::cerr << "Couldn't open shader file for loading: " << filePath << std::endl;
            return GL_NONE;
        }
        // read file contents into buffer
        std::stringstream buf;
        buf << file.rdbuf();
        std::string fileContents = buf.str();
        const char* fileCStr = fileContents.c_str();

        // create and compile shader from contents
        GLuint shader = glCreateShader(shaderType);
        glShaderSource(shader, 1, &fileCStr, nullptr);
        glCompileShader(shader);
        return shader;
    }

    bool Shader::IsShaderCompiled(GLuint shader) {
        GLint compileSucceeded = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compileSucceeded);
        if (compileSucceeded == GL_FALSE) {
            GLint errLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &errLen);

            GLchar* errLog = new GLchar[errLen];
            glGetShaderInfoLog(shader, errLen, &errLen, errLog);

            std::cerr << "Error compiling shader: " << errLog << std::endl;
            delete[] errLog;
            return false;
        }
        // shader is successfully compiled
        return true;
    }

    bool Shader::IsProgramLinked(GLuint program) {
        GLint linkSucceeded = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &linkSucceeded);
        if(linkSucceeded == GL_FALSE)
        {
            GLint errLen = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &errLen);

            GLchar* errLog = new GLchar[errLen];
            glGetProgramInfoLog(program, errLen, &errLen, errLog);

            std::cerr << "Error linking shader program: " << errLog << std::endl;
            delete[] errLog;
            return false;
        }
        // program is successfully linked
        return true;
    }

    bool Shader::IsActive() const {
        GLint id;
        glGetIntegerv(GL_CURRENT_PROGRAM, &id);
        return (id == this->mProgram);
    }

    GLuint Shader::GetProgramHandle() const { return mProgram; }
}
//
// Created by snaki on 12/6/2020.
//

#ifdef _WIN32
#include <windows.h>
#else
#define APIENTRY
#endif

#include <renderer/Renderer.h>

#include <CSCI441/objects.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//#define GL_DEBUG

namespace kVox {

    void APIENTRY
    MessageCallback(GLenum source,
    GLenum type,
            GLuint id,
    GLenum severity,
            GLsizei length,
    const GLchar* message,
    const void* userParam )
    {
        fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x\n\tmessage = %s\n",
        ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
        type, severity, message );
    }

    // utility uniform functions
// ------------------------------------------------------------------------
    void shaderSetBool(const GLuint program_id, const std::string &name, bool value) {
        glUniform1i(glGetUniformLocation(program_id, name.c_str()), (int)value);
    }
// ------------------------------------------------------------------------
    void shaderSetInt(const GLuint program_id, const std::string &name, int value) {
        glUniform1i(glGetUniformLocation(program_id, name.c_str()), value);
    }
// ------------------------------------------------------------------------
    void shaderSetFloat(const GLuint program_id, const std::string &name, float value) {
        glUniform1f(glGetUniformLocation(program_id, name.c_str()), value);
    }
// ------------------------------------------------------------------------
    void shaderSetVec2(const GLuint program_id, const std::string &name, const glm::vec2 &value) {
        glUniform2fv(glGetUniformLocation(program_id, name.c_str()), 1, &value[0]);
    }
    void shaderSetVec2(const GLuint program_id, const std::string &name, float x, float y) {
        glUniform2f(glGetUniformLocation(program_id, name.c_str()), x, y);
    }
// ------------------------------------------------------------------------
    void shaderSetVec3(const GLuint program_id, const std::string &name, const glm::vec3 &value) {
        glUniform3fv(glGetUniformLocation(program_id, name.c_str()), 1, &value[0]);
    }
    void shaderSetVec3(const GLuint program_id, const std::string &name, float x, float y, float z) {
        glUniform3f(glGetUniformLocation(program_id, name.c_str()), x, y, z);
    }
// ------------------------------------------------------------------------
    void shaderSetVec4(const GLuint program_id, const std::string &name, const glm::vec4 &value) {
        glUniform4fv(glGetUniformLocation(program_id, name.c_str()), 1, &value[0]);
    }
    void shaderSetVec4(const GLuint program_id, const std::string &name, float x, float y, float z, float w) {
        glUniform4f(glGetUniformLocation(program_id, name.c_str()), x, y, z, w);
    }
// ------------------------------------------------------------------------
    void shaderSetMat2(const GLuint program_id, const std::string &name, const glm::mat2 &mat) {
        glUniformMatrix2fv(glGetUniformLocation(program_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
// ------------------------------------------------------------------------
    void shaderSetMat3(const GLuint program_id, const std::string &name, const glm::mat3 &mat) {
        glUniformMatrix3fv(glGetUniformLocation(program_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
// ------------------------------------------------------------------------
    void shaderSetMat4(const GLuint program_id, const std::string &name, const glm::mat4 &mat) {
        glUniformMatrix4fv(glGetUniformLocation(program_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    bool Renderer::Init() {
        origin = new glm::vec3(0.0);
        // init video subsystem
        if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) { return false; }

        // create a window
        mWindow = SDL_CreateWindow("Stargazer", 600, 100, mWindowWidth, mWindowHeight, SDL_WINDOW_OPENGL);
        if (!mWindow) { return false; }

        // create OpenGL context
        mContext = SDL_GL_CreateContext(mWindow);
        if (mContext == nullptr) { return false; }

        // init OpenGL
        InitOpenGL();

        // initialize GLEW
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) { return false; }

#ifdef GL_DEBUG
        // debug output, comment as necessary
        glEnable              ( GL_DEBUG_OUTPUT );
        glDebugMessageCallback( MessageCallback, nullptr );
#endif

        // debug: init simple shaders
        // if (!InitShaders()) { return false; }
        if (!InitLightingShader()) { return false; }
        // debug: init simple triangle to viewport
//        float tri_verts[] = {
//                0.0f,  0.5f,  0.0f,     // top
//                0.5f, -0.5f,  0.0f,     // right
//                -0.5f, -0.5f,  0.0f     // left
//        };
//
//        // test triangle
//        drawables.insert( new VAO(tri_verts, 9) );

        // init succeeded
        return true;
    }

    void Renderer::InitOpenGL() {
        // tell SDL we want to use OpenGL 4.1
        // these attributes must be set before creating the window.
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

        // use double buffering
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        // require hardware acceleration.
        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

        // we request from OpenGL at least 8-bits per channel for the color buffer
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

        glEnable(GL_DEPTH_TEST);					       // enable depth testing
        glDepthFunc( GL_LESS );							   // use less than depth test

        glEnable(GL_BLEND);								   // enable blending
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // use one minus blending equation

//        glEnable(GL_CULL_FACE);
//        glCullFace(GL_FRONT);
//        glFrontFace(GL_CW);

        glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );	// set the clear color to black
    }

    bool Renderer::InitSimpleShader() {
        auto* mShader = new Shader("assets/shaders/simple.v.glsl", "assets/shaders/simple.f.glsl");
        if (!mShader->IsGood()) { return false; }

        shaders.emplace(std::make_pair("simple", mShader));

        return true;
    }

    struct VertexNormal {
        GLfloat x, y, z;
        GLfloat nx, ny, nz;
    };

    // helper for SetupSkybox
    GLuint loadCubemap(std::vector<std::string> faces) {
        GLuint texId;
        glGenTextures(1, &texId);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texId);

        int w, h, nrChannels;
        for (GLuint i = 0; i < faces.size(); i++) {
            unsigned char *data = stbi_load(faces[i].c_str(), &w, &h, &nrChannels, 0);
            if (data) {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                             0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                stbi_image_free(data);
            } else {
                fprintf(stderr, "Cubemap texture failed to load at path: %s", faces[i].c_str());
                stbi_image_free(data);
            }
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        return texId;
    }

    void Renderer::SetupSkybox() {
        std::vector<std::string> faces {
                "assets/skybox/posx.jpg",
                "assets/skybox/negx.jpg",
                "assets/skybox/posy.jpg",
                "assets/skybox/negy.jpg",
                "assets/skybox/posz.jpg",
                "assets/skybox/negz.jpg"
        };
        skyboxTexId = loadCubemap(faces);
    }

    bool Renderer::InitLightingShader() {
        // init textured phong shader
//        auto* mShader = new Shader("assets/shaders/phong.v.glsl", "assets/shaders/phong.f.glsl",\
                                    nullptr, nullptr, nullptr);
        auto* mShader = new Shader("assets/shaders/blinn.v.glsl", "assets/shaders/blinn.f.glsl",\
                                    nullptr, nullptr, "assets/shaders/blinn.g.glsl");
        if (!mShader->IsGood()) { return false; }
        shaders.emplace(std::make_pair("lighting", mShader));
        SetActiveShader("lighting");

        // assign uniforms, they get sent to this shader
        /*
         *  struct Light {
                int lightType;      // 0 - point light, 1 - directional light, 2 - spotlight
                vec3 lightPos;      // light position in world space
                vec3 lightDir;      // light direction in world space
                float lightCutoff;  // angle of our spotlight
                vec3 lightColor;    // light color
            };
         */

        // directional light direction and color
        auto dirLightColor = glm::vec3(1.0);
        auto dirLightDirection = glm::vec3(1.0, 1.0, 1.0);
        shaderSetInt(mShader->GetProgramHandle(), "lights[0].lightType", 1);
        shaderSetVec3(mShader->GetProgramHandle(), "lights[0].lightDir", dirLightDirection);
        shaderSetVec3(mShader->GetProgramHandle(), "lights[0].lightColor", dirLightColor);
        // point light positions and colors
        glm::vec3 pointLightPositions[] = {
                glm::vec3( 2.0,  1.0,  -2.0),
                glm::vec3(-12.0,  8.0, 8.0)
        };
        glm::vec3 pointLightColors[] = {
                glm::vec3(0.8, 0.0, 0.3),
                glm::vec3(0.0)
        };
        for (int i = 1; i < 2; i++) {
            shaderSetInt(mShader->GetProgramHandle(), std::string("lights[") + std::to_string(i) + std::string("].lightType"),
                         0);
            shaderSetVec3(mShader->GetProgramHandle(), std::string("lights[") + std::to_string(i) + std::string("].lightPos"),
                          pointLightPositions[i]);
            shaderSetVec3(mShader->GetProgramHandle(), std::string("lights[") + std::to_string(i) + std::string("].lightColor"),
                          pointLightColors[i]);
        }

        glEnableVertexAttribArray(mShader->attributes.vPos);
        glVertexAttribPointer(mShader->attributes.vPos, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormal), nullptr);
        // enable the vertex normal attribute and specify the pointer to the data
        glEnableVertexAttribArray(mShader->attributes.vNorm);
        glVertexAttribPointer(mShader->attributes.vNorm, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormal), (void*)(sizeof(GLfloat)*3));

        mShader = new Shader("assets/shaders/skybox.v.glsl", "assets/shaders/skybox.f.glsl");
        if (!mShader->IsGood()) { return false; }
        shaders.emplace(std::make_pair("skybox", mShader));

        ////////////////////////////////////////////
        /// next: skybox
        GLfloat skyboxVertices[] = {
                // positions
                -1.0f,  1.0f, -1.0f,
                -1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,
                1.0f,  1.0f, -1.0f,
                -1.0f,  1.0f, -1.0f,

                -1.0f, -1.0f,  1.0f,
                -1.0f, -1.0f, -1.0f,
                -1.0f,  1.0f, -1.0f,
                -1.0f,  1.0f, -1.0f,
                -1.0f,  1.0f,  1.0f,
                -1.0f, -1.0f,  1.0f,

                1.0f, -1.0f, -1.0f,
                1.0f, -1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f,  1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,

                -1.0f, -1.0f,  1.0f,
                -1.0f,  1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f, -1.0f,  1.0f,
                -1.0f, -1.0f,  1.0f,

                -1.0f,  1.0f, -1.0f,
                1.0f,  1.0f, -1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                -1.0f,  1.0f,  1.0f,
                -1.0f,  1.0f, -1.0f,

                -1.0f, -1.0f, -1.0f,
                -1.0f, -1.0f,  1.0f,
                1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,
                -1.0f, -1.0f,  1.0f,
                1.0f, -1.0f,  1.0f
        };
        glGenVertexArrays(1, &skyboxVAO);
        glGenBuffers(1, &skyboxVBO);
        glBindVertexArray(skyboxVAO);
        glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
        SetupSkybox(); // make sure skybox obj handles are set up

        // next, let's set up our post-processing shader
        auto* ppShader = new Shader("assets/shaders/pp.v.glsl", "assets/shaders/pp.f.glsl");
        if (!ppShader->IsGood()) { return false; }
        shaders.emplace(std::make_pair("post", ppShader));
        SetActiveShader("post");
        // initialize render/frame buffer objects
        glGenFramebuffers(1, &FBO);
        glGenRenderbuffers(1, &RBO);
        // init RBO storage with multisampled color buffer (don't need depth/stencil buffer)
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        glBindRenderbuffer(GL_RENDERBUFFER, RBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, mWindowWidth, mWindowHeight); // allocate storage
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RBO); // attach MS RBO to FBO
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWindowWidth, mWindowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            return false;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        shaderSetInt(ppShader->GetProgramHandle(), "scene", 0);
        float offset = 1.0f/300.0f;
        float offsets[9][2] = {
                { -offset,  offset  },  // top-left
                {  0.0f,    offset  },  // top-center
                {  offset,  offset  },  // top-right
                { -offset,  0.0f    },  // center-left
                {  0.0f,    0.0f    },  // center-center
                {  offset,  0.0f    },  // center - right
                { -offset, -offset  },  // bottom-left
                {  0.0f,   -offset  },  // bottom-center
                {  offset, -offset  }   // bottom-right
        };
        glUniform2fv(glGetUniformLocation(ppShader->GetProgramHandle(), "offsets"), 9, (float*)offsets);
        int edge_kernel[9] = {
                -1, -1, -1,
                -1,  8, -1,
                -1, -1, -1
        };
        glUniform1iv(glGetUniformLocation(ppShader->GetProgramHandle(), "edge_kernel"), 9, edge_kernel);
        float blur_kernel[9] = {
                1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f,
                2.0f / 16.0f, 4.0f / 16.0f, 2.0f / 16.0f,
                1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f
        };
        glUniform1fv(glGetUniformLocation(ppShader->GetProgramHandle(), "blur_kernel"), 9, blur_kernel);
        // setup screen quad
        float vertices[] = {
                // pos        // tex
                -1.0f, -1.0f, 0.0f, 0.0f,
                1.0f,  1.0f, 1.0f, 1.0f,
                -1.0f,  1.0f, 0.0f, 1.0f,

                -1.0f, -1.0f, 0.0f, 0.0f,
                1.0f, -1.0f, 1.0f, 0.0f,
                1.0f,  1.0f, 1.0f, 1.0f
        };
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);

        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindVertexArray(quadVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        return true;
    }

    void Renderer::Shutdown() {
        // clean up drawables that were left to us
        for (const auto& drawable : drawables) {
            for (VAO *vao : drawable.second) {
                delete vao;
            }
        }
        drawables.clear();
        // clean up cameras that were left to us
        for (const auto& camera : cameras) {
            delete camera.second;
        }
        cameras.clear();
        // clean up shaders that were left to us
        for (const auto & shader : shaders) {
            delete shader.second;
        }
        shaders.clear();
        // delete library VBOs/VAOs
        CSCI441::deleteObjectVBOs();
        CSCI441::deleteObjectVAOs();

        // clean up our origin
        delete origin;

        if (mWindow != nullptr) {
            SDL_DestroyWindow(mWindow);
        }
    }

    void Renderer::Clear() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void Renderer::BeginRender() {
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        glDrawBuffer(GL_BACK);
        glClearColor(0.0,0.0,0.0,1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void Renderer::EndRender() {
        glFlush(); // make sure that OpenGL rendered everything
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // detach fbo
    }

    void Renderer::Render(double deltaTime, bool mRunning) {
        this->BeginRender();
        this->activeCamera->RecomputeCamPos();
        ////////// ** BEGIN RENDER STAGE ** //////////
        if (!mRunning)
            return;
        // get true framebuffer size
        GLint framebufferWidth, framebufferHeight;
        SDL_GetWindowSize( mWindow, &framebufferWidth, &framebufferHeight );
        // update viewport
        glViewport(0, 0, framebufferWidth, framebufferHeight);
        // update projection matrix based on size
        glm::mat4 projMtx = glm::perspective( 45.0f, (GLfloat)mWindowWidth / (GLfloat)mWindowHeight, 0.001f, 40000.0f);
        // set up lookAt matrix to position active camera (up is positive y-axis)
        glm::mat4 viewMtx = glm::lookAt(activeCamera->camPos, activeCamera->camLookAt, glm::vec3(0,1,0));

        for (const auto & drawable : drawables) {
            // for draw batch, activate shader if not done
            if (drawable.first != _activeShader)
                SetActiveShader(drawable.first);
            // then, draw all drawables in this batch
            for (VAO* vao : drawable.second) {
                // update shader's uniforms as necessary
                UpdateShaderUniforms(vao->GetModelMtx(), viewMtx, projMtx);
                glUniform3fv(shaders.at(_activeShader)->uniforms.materialAmbColor,  1, &(vao->material.materialAmbColor[0]));
                glUniform3fv(shaders.at(_activeShader)->uniforms.materialDiffColor, 1, &(vao->material.materialDiffColor[0]));
                glUniform3fv(shaders.at(_activeShader)->uniforms.materialSpecColor, 1, &(vao->material.materialSpecColor[0]));
                glUniform1f(shaders.at(_activeShader)->uniforms.materialShininess, vao->material.materialShininess);
                vao->Draw();
            }
        }
        /// last thing to do: render skybox
        glDepthFunc(GL_LEQUAL);
        SetActiveShader("skybox");
        glm::mat4 skyview = glm::mat4(glm::mat3(viewMtx)); // remove translation data from view matrix
        shaderSetMat4(shaders.at(_activeShader)->GetProgramHandle(), "view", skyview);
        shaderSetMat4(shaders.at(_activeShader)->GetProgramHandle(), "projection", projMtx);
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexId);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);
        ////////// ** END RENDER STAGE ** //////////
        this->EndRender();
        cumulativePostTime += deltaTime;
        // time to handle post-processing
        SetActiveShader("post");
        GLuint postShaderID = shaders.at(_activeShader)->GetProgramHandle();
        shaderSetFloat(postShaderID, "time", static_cast<float>(cumulativePostTime));
        shaderSetInt(postShaderID, "confuse", confuse);
        shaderSetInt(postShaderID, "chaos", chaos);
        shaderSetInt(postShaderID, "shake", shake);
        // render textured quad
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

    void Renderer::Swap() {
        SDL_GL_SwapWindow(mWindow);
    }

    void Renderer::AddDrawable(VAO* obj) {
        std::string shaderName = obj->GetShader();
        if (!drawables.contains(shaderName)) {
            // init new draw batch set if we don't render any drawables with this shader
            drawables[shaderName] = std::set<VAO*>();
        }
        drawables.at(shaderName).insert(obj);
    }
    void Renderer::RemoveDrawable(VAO* obj) {
        // we are guaranteed to exist in the map/set, so get and remove the obj by shader name
        std::string shaderName = obj->GetShader();
        assert(drawables.contains(shaderName));
        drawables.at(shaderName).erase(obj);
    }

    void Renderer::AddShader(const std::string& name, Shader* shader) {
        shaders[name] = shader;
    }
    void Renderer::RemoveShader(const std::string &name) {
        if (!shaders.contains(name)) { return; }
        shaders.erase(name);
    }

    Shader* Renderer::GetShaderWithName(const std::string &name) {
        if (shaders.contains(name))
            return shaders.at(name);
        else
            return nullptr;
    }

    std::string Renderer::ActiveShader() { return _activeShader; }

    void Renderer::SetActiveShader(const std::string &name) {
        _activeShader = name;
        assert(shaders.contains(name));
        shaders.at(name)->Activate();
        GLint vPosLoc = shaders.at(name)->attributes.vPos;
        GLint vNormLoc = shaders.at(name)->attributes.vNorm;
        CSCI441::setVertexAttributeLocations(vPosLoc, vNormLoc);
    }

    // update what shader uniforms we can
    void Renderer::UpdateShaderUniforms(glm::mat4 modelMtx, glm::mat4 viewMtx, glm::mat4 projMtx) {
//        struct ShaderUniforms {         // stores the locations of all of our shader uniforms
//          / GLint mvpMatrix;                    // the MVP Matrix to apply
//          / GLint modelMatrix;                  // model matrix
//          / GLint normalMtx;                    // normal matrix
//          / GLint eyePos;                       // camera position
//          X GLint materialDiffColor;            // material diffuse color
//          X GLint materialSpecColor;            // material specular color
//          X GLint materialShininess;            // material shininess factor
//          X GLint materialAmbColor;             // material ambient color
//        };
        glm::mat4 mvpMtx = projMtx * viewMtx * modelMtx;
        glm::mat4 mvMtx = viewMtx * modelMtx;
        glUniformMatrix4fv(shaders.at(_activeShader)->uniforms.mvpMatrix, 1, GL_FALSE, &mvpMtx[0][0]);
        glUniformMatrix4fv(shaders.at(_activeShader)->uniforms.mvMatrix, 1, GL_FALSE, &mvMtx[0][0]);
        glUniformMatrix4fv(shaders.at(_activeShader)->uniforms.modelMatrix, 1, GL_FALSE, &modelMtx[0][0]);
        glUniformMatrix4fv(shaders.at(_activeShader)->uniforms.viewMtx, 1, GL_FALSE, &viewMtx[0][0]);
        glUniformMatrix4fv(shaders.at(_activeShader)->uniforms.projMtx, 1, GL_FALSE, &projMtx[0][0]);
        glm::mat4 normalMtx = glm::transpose( glm::inverse( modelMtx ) );
//        std::cout << (glm::to_string(normalMtx)) << std::endl;
        glUniformMatrix4fv(shaders.at(_activeShader)->uniforms.normalMtx, 1, GL_FALSE, &normalMtx[0][0]);
        glUniform3fv(shaders.at(_activeShader)->uniforms.eyePos, 1, &(activeCamera->camPos[0]));
    }

    void Renderer::AddCamera(const std::string &name, Camera *camera) {
        cameras[name] = camera;
    }
    void Renderer::RemoveCamera(const std::string &name) {
        cameras.erase(name);
    }
    Camera* Renderer::ActiveCamera() { return activeCamera; }
    void Renderer::SetActiveCamera(const std::string &name) {
        activeCamera = cameras.at(name);
    }
    Camera* Renderer::GetCameraWithName(const std::string &name) {
        if (cameras.contains(name))
            return cameras.at(name);
        else
            return nullptr;
    }

    const glm::vec3* Renderer::GetOrigin() { return origin; }

    int Renderer::GetWindowWidth() const { return mWindowWidth; }
    int Renderer::GetWindowHeight() const { return mWindowHeight; }

    void Renderer::UpdateShaderFloat(const std::string &shader, const std::string &attr, double val) {
        shaderSetFloat(shaders.at(shader)->GetProgramHandle(), attr, static_cast<float>(val));
    }

    void Renderer::SetShake(bool set) { this->shake = set; }
    void Renderer::SetChaos(bool set) { this->chaos = set; }
    void Renderer::SetConfuse(bool set) { this->confuse = set; }
}
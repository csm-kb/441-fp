//
// Created by snaki on 12/6/2020.
//

#ifndef FP_RENDERER_H
#define FP_RENDERER_H

#include <SDL2/SDL.h>
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <CSCI441/OpenGLUtils.hpp>
#include <CSCI441/objects.hpp>  // for our 3D objects
#include <CSCI441/TextureUtils.hpp>

#include <map>
#include <set>
#include <vector>
#include <string>

#include <renderer/Shader.h>
#include <renderer/Camera.h>

namespace kVox {
    class VAO;

    class Renderer {
    public:
        /** Initializes the renderer. To be called only once on engine load. */
        bool Init();

        /** Shuts down the renderer. To be called only once on engine shutdown. */
        void Shutdown();

        /** Performs OpenGL clear tasks, as necessary. */
        void Clear();

        /**
         * The core OpenGL rendering loop.
         * @param deltaTime : the amount of time passed since the last frame
         * @param mRunning : whether the game engine is still running (for double-checking)
         */
        void Render(double deltaTime, bool mRunning);

        /** Swaps the graphics engine's draw buffers, if double-buffering is supported/enabled. */
        void Swap();

        /** Adds a drawable object to the render queue. */
        void AddDrawable(VAO* obj);
        void RemoveDrawable(VAO* obj);
        /** Adds a shader object to the shader map. */
        void AddShader(const std::string& name, Shader* shader);
        void RemoveShader(const std::string& name);

        /** Obtains the shader with the given name, or \c nullptr if it doesn't exist. */
        Shader* GetShaderWithName(const std::string& name);
        /** Obtains the name of the currently-active shader. */
        std::string ActiveShader();
        /** Sets the active shader to be the one with the specified name. */
        void SetActiveShader(const std::string& name);
        /** Passes all necessary values over to the active shader. */
        void UpdateShaderUniforms(glm::mat4 modelMtx, glm::mat4 viewMtx, glm::mat4 projMtx);

        // camera functions
        /** Adds a camera object to the camera map. */
        void AddCamera(const std::string& name, Camera* camera);
        void RemoveCamera(const std::string& name);
        /** Obtains a reference to the currently-active camera. */
        Camera* ActiveCamera();
        /** Sets the active camera to be the one with the specified name. */
        void SetActiveCamera(const std::string& name);
        /** Obtains the camera with the given name, or \c nullptr if it doesn't exist. */
        Camera* GetCameraWithName(const std::string& name);

        /** Obtains the current window width. */
        int GetWindowWidth() const;
        /** Obtains the current window height. */
        int GetWindowHeight() const;

        /** Updates a shader's float attribute with the specified name to the supplied value. */
        void UpdateShaderFloat(const std::string& shader, const std::string& attr, double val);

        /** Toggles the 'shake' post-processing effect. */
        void SetShake(bool set);
        /** Toggles the 'chaos' post-processing effect. */
        void SetChaos(bool set);
        /** Toggles the 'confuse' post-processing effect. */
        void SetConfuse(bool set);

        /** Retrieves the origin of the render world space. */
        const glm::vec3* GetOrigin();

    private:

        /**
         * Initialize OpenGL configuration and runtime for the renderer
         */
        void InitOpenGL();

        /**
         * Initialize a simple shader for the renderer
         * @return whether the shader was successfully initialized
         */
        bool InitSimpleShader();

        /**
         * Initialize a Gourad lighting shader for the renderer
         * @return whether the shader was successfully initialized
         */
        bool InitLightingShader();

        /**
         * Handle for the window.
         */
        SDL_Window *mWindow = nullptr;
        GLint mWindowWidth = 1024, mWindowHeight = 768;

        /**
         * OpenGL rendering context.
         */
        SDL_GLContext mContext;

        std::map<std::string, Shader*> shaders;
        std::string _activeShader;

        // shader-sorted mapping of drawables
        std::map<std::string, std::set<VAO*>> drawables;

        std::map<std::string, Camera*> cameras;
        Camera* activeCamera;

        glm::vec3* origin;

        // post-processing details
        GLuint texture;
        bool confuse, chaos, shake;
        double cumulativePostTime = 0.0;
        // framebuffer handles
        GLuint FBO, RBO, quadVAO, quadVBO;
        void BeginRender();
        void EndRender();
        // skybox object handles
        GLuint skyboxVAO, skyboxVBO, skyboxTexId;
        void SetupSkybox();
    };

    /**
     * Basic data structure for holding material ambient, diffuse, and specular properties.
     */
    struct VAOMatProps {
        glm::vec3 materialDiffColor; // material diffuse color
        glm::vec3 materialSpecColor; // material specular color
        glm::vec3 materialAmbColor;  // material ambient color
        double materialShininess; // material shininess factor
    };

    /**
     * The basic drawable. Stores values pertinent to the renderer's OpenGL pipeline, as well as
     * which shader to render with, which model matrix to render (typically set by a game object),
     * and material properties to render with.
     */
    class VAO {
    public:
        VAO() = delete;
        VAO(const float *vertPos, int vertPosCount, const std::string &shaderToUse, Renderer &renderer);
        ~VAO();

        virtual void Draw() const;
        void SetShader(const std::string& shaderName);
        std::string GetShader();

        glm::mat4 GetModelMtx();
        void SetModelMtx(glm::mat4 modelMtx);

        inline bool operator==(VAO&a) {
            return (this->mVAO == a.mVAO && this->mVBO == a.mVAO && this->mVertCount == a.mVertCount);
        }

        VAOMatProps material;

    protected:
        /** vertex array object handle */
        GLuint mVAO = GL_NONE;
        /** vertex buffer object handle */
        GLuint mVBO = GL_NONE;
        /** model matrix, if model has one */
        glm::mat4 modelMtx = glm::mat4(1.0);

        /** renderer handle */
        Renderer& renderer;

        std::string shaderToRenderWith = "simple";

        int mVertCount = 0;
    };

    /**
     * Primitive type specifiers, for use by \c PrimitiveVAO objects.
     */
    enum PrimitiveType {
        CUBE, CONE, CYLINDER, TORUS, SPHERE
    };

    /**
     * A derivative of the base drawable that supports the CSCI441 \c Objects.hpp library.
     */
    class PrimitiveVAO : public VAO {
    public:
        PrimitiveVAO() = delete;
        PrimitiveVAO(const float *vertPos, int vertPosCount, const std::string &shaderToUse, Renderer &renderer, PrimitiveType type)
            : VAO(vertPos, vertPosCount, shaderToUse, renderer), primitive(type) { primitive = type; }

        void Draw() const override;
    private:
        PrimitiveType primitive;
    };
}

#endif //FP_RENDERER_H

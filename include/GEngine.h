//
// Created by snaki on 12/6/2020.
//

#ifndef FP_GENGINE_H
#define FP_GENGINE_H

#include <memory>
#include <vector>
#include <set>

#include <SDL2/SDL.h>
#include <renderer/Renderer.h>
#include <kInputListener.h>
#include <kAnimHandler.h>

namespace kVox {

    class PlayerGO;
    class GEngine;
    typedef std::unique_ptr<GEngine> ENGINE_PTR;

    /**
     * The basic game object class within GEngine. <br>
     * Keeps track of its own transforms, drawable, parent, and children (if any).
     */
    class GObject {
        friend class GEngine;
    public:
        GObject() = delete;
        explicit GObject(const Renderer& renderer);
        ~GObject();

        std::string name;

        void SetVAO(VAO* vao);

        void SetPosition(glm::vec3 pos, bool local=false);
        void SetPosition(double x, double y, double z, bool local=false);
        void SetRotation(glm::vec3 rotEuler, bool local);
        void SetRotation(double x, double y, double z, bool local=false);
        void SetScale(glm::vec3 scale, bool local=false);
        void SetScale(double x, double y, double z, bool local=false);
        void SetVelocity(glm::vec3 vel);
        void SetVelocity(double x, double y, double z);

        glm::vec3 GetPosition();
        glm::vec3 GetVelocity();
        glm::vec3*GetPosAsPtr();
        glm::vec3 GetRotEuler();
        glm::quat GetRotation();
        glm::vec3 GetScale();
        glm::vec3 GetOrientation();
        glm::vec3 GetOrientWithPos();
        glm::vec3*GetOrientAsPtr();
        glm::vec3 GetForwardDef();

        glm::vec3 GetLocalPos();
        glm::vec3*GetLocalPosPtr();
        glm::vec3 GetLocalRotEuler();
        glm::quat GetLocalRot();
        glm::vec3 GetLocalScale();

        /** Called by the game engine every Update cycle. */
        virtual void Update();

        /** Enable physics for this object. */
        void EnablePhys();
        /** Disable physics for this object. */
        void DisablePhys();
        /** Called by the game engine's physics handler every update. Only impacts phys-enabled objects. */
        void PhysUpdate(double deltaTime);

        /** Updates this object's model matrix, given its position, rotation, and scale. */
        void UpdateModelMtx();
        /** To be called by a parent GObject, if any */
        void UpdateModelMtx(glm::mat4 parentModelMtx);

        GObject* parent = nullptr;
        std::set<GObject*> children;

    protected:
        bool isPhysEnabled = false;
        glm::vec3 pos;      // position (x,y,z) in cartesian coordinates
        glm::vec3 vel;      // velocity (x,y,z) in cartesian coordinates
        glm::vec3 rot;      // rotation (x,y,z) in Euler angles
        glm::vec3 scale;    // scale    (x,y,z) in multiples
        glm::vec3 forward = glm::vec3(0.0,0.0,1.0); // default forward vector
        glm::vec3 orient;   // updated constantly from rot + pos
        glm::vec3 localPos, localRot, localScale = glm::vec3(0.0);
        const Renderer& renderer;
        VAO* vao;
    };

    class EnemyGO : public GObject {
    public:
        EnemyGO() = delete;
        explicit EnemyGO(const Renderer& renderer);

        void Update() override;
    private:
        void PlayerCollisionCheck();

        const std::string playerObj = "torus";
        GObject* player;
    };

    class GoalGO : public GObject {
    public:
        GoalGO() = delete;
        explicit GoalGO(const Renderer& renderer);

        void Update() override;
    private:
        void PlayerCollisionCheck();

        const std::string playerObj = "torus";
        PlayerGO* player;
    };

    class PlayerGO : public GObject {
        friend class GoalGO;
    public:
        PlayerGO() = delete;
        explicit PlayerGO(const Renderer& renderer);

        void Update() override;
    private:
        const int goalsToWin = 3;
        int goalCount = 0;
    };

    /**
     * The game engine class, singleton-style (by necessity).<br>
     * Keeps track of all engine-related activities, and is responsible for the core game/render loop.
     */
    class GEngine {
    public:
        static GEngine& Instance() {
            if (engine == nullptr) {
                engine.reset(new GEngine());
            }
            return *engine;
        };
        /**
         * Initializes the engine.
         * @return whether engine was successfully initialized
         */
        bool Init();

        /**
         * Shuts down the engine.
         */
        void Shutdown();

        /**
         * Runs the game loop until done.
         */
        void Run();

        /**
         * Obtains the renderer for the game. <br><br>
         * TODO: replace with service locator
         */
        Renderer& GetRenderer();

        // game object handles
        /** Returns the \c GObject handle with the given name, or \c nullptr if it doesn't exist. */
        GObject* GetGameObject(const std::string& name);
        /** Adds a \c GObject with the given name.<br>
         *  Returns \c true if successful, or \c false if an object with that name already exists. */
        bool AddGameObject(const std::string& name, GObject* gameObject);
        /** Removes the \c GObject with the given name.<br>
         *  Returns \c true if successful, or \c false if an object with that name doesn't exist. */
        bool RemoveGameObject(const std::string& name);

        // input listener registers
        /** Register a key input listener with the engine. */
        void RegisterKeyInputListener(const std::shared_ptr<kKeyInputListener>& listener);
        /** Register a mouse button listener with the engine. */
        void RegisterMouseButtonListener(const std::shared_ptr<kMouseButtonListener>& listener);
        /** Register a mouse motion listener with the engine. */
        void RegisterMouseMotionListener(const std::shared_ptr<kMouseMotionListener>& listener);
        /** Unregister a key input listener from the engine. */
        void UnregisterKeyInputListener(const std::shared_ptr<kKeyInputListener>& listener);
        /** Unregister a mouse button listener from the engine. */
        void UnregisterMouseButtonListener(const std::shared_ptr<kMouseButtonListener>& listener);
        /** Unregister a mouse motion listener from the engine. */
        void UnregisterMouseMotionListener(const std::shared_ptr<kMouseMotionListener>& listener);

        // GO animation registers
        /** Register an animation with the engine, to be handled every update. */
        void RegisterAnim(const std::shared_ptr<kAnimHandler>& handler);
        /** Unregister an animation from the engine. */
        void UnregisterAnim(const std::shared_ptr<kAnimHandler>& handler);

        // keyboard state getter
        bool IsKeyPressed(uint32_t key);

        // mouse button state getter
        bool IsMouseButtonPressed(uint32_t button);

        GEngine(GEngine const&)         = delete;
        void operator=(GEngine const&)  = delete;

    private:
        static ENGINE_PTR engine;
        GEngine() {}

        /** Handles the engine's core game loop */
        bool mRunning = false;
        /** The renderer core that powers the game engine */
        Renderer mRenderer;

        /** Measured delta time of the game loop, in seconds per frame */
        double mDeltaTime = 0.016;

        // input handler
        bool KEYS[322] = {false};
        bool MOUSE[4] = {false};

        /** The 'scene' -- keeps track of all game objects by name. */
        std::map<std::string,GObject*> mGameObjects;

        /** High-level listeners (e.g. input) from other components of the game */
        //
        std::vector< std::shared_ptr<kKeyInputListener> >    keyInputListeners;
        std::vector< std::shared_ptr<kMouseButtonListener> > mouseButtonListeners;
        std::vector< std::shared_ptr<kMouseMotionListener> > mouseMotionListeners;

        /** High-level animation listener for game objects */
        std::set< std::shared_ptr<kAnimHandler> > goAnimHandlers;
        void HandleAnims(double deltaTime);

        /** Simple physics handling system for game objects */
        void HandlePhys(double deltaTime);

        // three phases of a game loop
        /**
         * Process engine inputs.
         */
        void ProcessInput();

        /**
         * Updates the engine's simulation.
         */
        void Update();

        /**
         * Generates outputs.
         */
        void GenerateOutputs();
    };
}

#endif //FP_GENGINE_H

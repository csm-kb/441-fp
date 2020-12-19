//
// Created by snaki on 12/6/2020.
//

#include <GEngine.h>
#include <iostream>
#include <glm/gtx/string_cast.hpp>
#include <stdio.h>

namespace kVox {
    ENGINE_PTR GEngine::engine = nullptr;

    bool GEngine::Init() {
        // initialize the renderer
        mRenderer = Renderer();
        return mRenderer.Init();
    }

    void GEngine::Shutdown() {
        mRunning = false;

        // destroy listeners left to us
        for (const auto& listener : keyInputListeners) {
            delete listener.get();
        }
        keyInputListeners.clear();
        for (const auto& listener : mouseButtonListeners) {
            delete listener.get();
        }
        mouseButtonListeners.clear();
        for (const auto& listener : mouseMotionListeners) {
            delete listener.get();
        }
        mouseMotionListeners.clear();
        for (const auto& handler : goAnimHandlers) {
            delete handler.get();
        }

        // destroy renderer
        mRenderer.Shutdown();

        // destroy game objects left
        for (const auto& gObj : mGameObjects) {
            delete gObj.second;
        }
        mGameObjects.clear();

        SDL_Quit();
    }

    void GEngine::Run() {
        // engine is running
        mRunning = true;

        // game loop until done
        while (mRunning) {
            ProcessInput();
            if (!mRunning) break;
            Update();
            GenerateOutputs();
        }
    }

    void GEngine::ProcessInput() {
        // poll for events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT: {
                    mRunning = false;
                    break;
                }
                case SDL_KEYDOWN:{
                    // handle key down input
                    if (event.key.keysym.sym < 323)
                        KEYS[event.key.keysym.sym] = true;
                    for (const auto& listener : keyInputListeners) {
                        listener->update(true, event.key);
                    }
                    break;
                }
                case SDL_KEYUP: {
                    // handle key up input
                    if (event.key.keysym.sym < 323)
                        KEYS[event.key.keysym.sym] = false;
                    for (const auto& listener : keyInputListeners) {
                        listener->update(false, event.key);
                    }
                    break;
                }
                case SDL_MOUSEBUTTONDOWN: {
                    // handle mouse button down
                    MOUSE[event.button.button] = true;
                    for (const auto& listener : mouseButtonListeners) {
                        listener->update(event.button);
                    }
                    break;
                }
                case SDL_MOUSEBUTTONUP: {
                    // handle mouse button up
                    MOUSE[event.button.button] = false;
                    for (const auto& listener : mouseButtonListeners) {
                        listener->update(event.button);
                    }
                    break;
                }
                case SDL_MOUSEMOTION: {
                    // handle mouse motion
                    for (const auto& listener : mouseMotionListeners) {
                        listener->update(event.motion);
                    }
                    break;
                }
            }
        }
    }

    void GEngine::Update() {
        // tracks last tick value each time loop is run
        static uint64_t lastTicks = 0;
        // get current tick value (milliseconds)
        uint64_t currentTicks = SDL_GetPerformanceCounter();
        // calculate change from current to last, converting to seconds (in float)
        mDeltaTime = ((currentTicks - lastTicks) / (double)SDL_GetPerformanceFrequency() );
        // save ticks value for next frame
        lastTicks = currentTicks;
        // ensure delta time is never negative
        if (mDeltaTime < 0.0) { mDeltaTime = 0.0; }
        // limit the time delta to 0.05 seconds (about 20 FPS).
        if (mDeltaTime > 0.05) { mDeltaTime = 0.05; }

        // have all game objects update
        for (const auto& go : mGameObjects) {
            go.second->Update();
        }
        // next, handle any registered animations for game objects
        HandleAnims(mDeltaTime);
        // next, handle physics for phys-enabled game objects
        HandlePhys(mDeltaTime);

        // debug: print FPS to console
        printf("\rFPS: %f", 1.0 / mDeltaTime);
        fflush(stdout);
    }

    void GEngine::HandlePhys(double deltaTime) {
        for (const auto& it : mGameObjects) {
            it.second->PhysUpdate(deltaTime);
        }
    }

    void GEngine::HandleAnims(double deltaTime) {
        for (const auto& handler : goAnimHandlers) {
            handler->update(deltaTime);
        }
    }
    void GEngine::RegisterAnim(const std::shared_ptr<kAnimHandler>& handler) {
        goAnimHandlers.insert(handler);
    }
    void GEngine::UnregisterAnim(const std::shared_ptr<kAnimHandler>& handler) {
        goAnimHandlers.erase(handler);
    }

    void GEngine::GenerateOutputs() {
        mRenderer.Clear();
        mRenderer.Render(mDeltaTime, mRunning);
        mRenderer.Swap();
    }

    void GEngine::RegisterKeyInputListener(const std::shared_ptr<kKeyInputListener>& listener) { keyInputListeners.emplace_back(listener); }
    void GEngine::RegisterMouseButtonListener(const std::shared_ptr<kMouseButtonListener>& listener) { mouseButtonListeners.emplace_back(listener); }
    void GEngine::RegisterMouseMotionListener(const std::shared_ptr<kMouseMotionListener>& listener) { mouseMotionListeners.emplace_back(listener); }

    void GEngine::UnregisterKeyInputListener(const std::shared_ptr<kKeyInputListener>& listener) {
        auto _pos = keyInputListeners.cend();
        for (auto it = keyInputListeners.cbegin(); it != keyInputListeners.cend(); ++it) {
            if (*it == listener) {
                _pos = it;
                break;
            }
        }
        if (_pos == keyInputListeners.cend())
            return;
        keyInputListeners.erase(_pos);
    }
    void GEngine::UnregisterMouseButtonListener(const std::shared_ptr<kMouseButtonListener>& listener) {
        auto _pos = mouseButtonListeners.cend();
        for (auto it = mouseButtonListeners.cbegin(); it != mouseButtonListeners.cend(); ++it) {
            if (*it == listener) {
                _pos = it;
                break;
            }
        }
        if (_pos == mouseButtonListeners.cend())
            return;
        mouseButtonListeners.erase(_pos);
    }
    void GEngine::UnregisterMouseMotionListener(const std::shared_ptr<kMouseMotionListener>& listener) {
        auto _pos = mouseMotionListeners.cend();
        for (auto it = mouseMotionListeners.cbegin(); it != mouseMotionListeners.cend(); ++it) {
            if (*it == listener) {
                _pos = it;
                break;
            }
        }
        if (_pos == mouseMotionListeners.cend())
            return;
        mouseMotionListeners.erase(_pos);
    }

    bool GEngine::IsKeyPressed(uint32_t key) {
        assert(key > 0 && key < 322);
        return KEYS[key];
    }

    bool GEngine::IsMouseButtonPressed(uint32_t button) {
        assert (button > 0 && button < 4);
        return MOUSE[button];
    }

    Renderer& GEngine::GetRenderer() {
        return mRenderer;
    }

    GObject* GEngine::GetGameObject(const std::string& name) {
        if (mGameObjects.contains(name))
            return mGameObjects.at(name);
        else
            return nullptr;
    }

    bool GEngine::AddGameObject(const string &name, GObject *gameObject) {
        if (!mGameObjects.contains(name)) {
            mGameObjects[name] = gameObject;
            gameObject->name = name;
            return true;
        } else {
            return false;
        }
    }

    bool GEngine::RemoveGameObject(const string &name) {
        if (mGameObjects.contains(name)) {
            mGameObjects.erase(name);
            return true;
        } else {
            return false;
        }
    }

    GObject::GObject(const Renderer &renderer) : renderer(renderer) {
        this->pos = glm::vec3(0.0);
        this->rot = glm::vec3(0.0);
        this->scale = glm::vec3(1.0);
    }
    GObject::~GObject() {
        GEngine& engine = GEngine::Instance();
        // remove itself from attached game objects (will sever links!)
        engine.RemoveGameObject(this->name);
        if (this->parent != nullptr) {
            this->parent->children.erase(this);
        }
        for (auto* child : this->children) {
            child->parent = nullptr;
        }
        // clean up the object's vao, if any
        VAO* vao = this->vao;
        if (vao != nullptr) {
            this->vao = nullptr;
            engine.GetRenderer().RemoveDrawable(vao);
            vao->SetShader("__NULL_SHADER");
            delete vao;
        }
    }

    void GObject::SetVAO(VAO* vao) { this->vao = vao; }

    void GObject::SetPosition(glm::vec3 pos, bool local) {
        this->pos = pos; this->UpdateModelMtx();
    }
    void GObject::SetPosition(double x, double y, double z, bool local) {
        this->SetPosition(glm::vec3(x,y,z),local);
    }
    void GObject::SetRotation(glm::vec3 rotEuler, bool local) {
        this->rot = rotEuler; this->UpdateModelMtx();
    }
    void GObject::SetRotation(double x, double y, double z, bool local) {
        this->SetRotation(glm::vec3(x,y,z),local);
    }
    void GObject::SetScale(glm::vec3 scale, bool local) {
        this->scale = scale; this->UpdateModelMtx();
    }
    void GObject::SetScale(double x, double y, double z, bool local) {
        this->SetScale(glm::vec3(x,y,z),local);
    }
    void GObject::SetVelocity(glm::vec3 vel) {
        this->vel = vel;
    }
    void GObject::SetVelocity(double x, double y, double z) {
        this->SetVelocity(glm::vec3(x,y,z));
    }

    bool IsVec3InTolerance(glm::vec3 vec, double epsilon) {
        assert(epsilon > 0.0);
        return (glm::abs(vec.x) <= epsilon && glm::abs(vec.y) <= epsilon && glm::abs(vec.z) <= epsilon);
    }

    void GObject::EnablePhys() {
        this->isPhysEnabled = true;
        this->vel = glm::vec3(0.0); // reset velocity as a safety measure
    }
    void GObject::DisablePhys() {
        this->isPhysEnabled = false;
    }
    void GObject::PhysUpdate(double deltaTime) {
        if (!isPhysEnabled || IsVec3InTolerance(this->vel,0.01)) return;
        this->pos += this->vel * glm::vec3(deltaTime);
        this->UpdateModelMtx();
    }

    glm::vec3 GObject::GetPosition() { return this->pos; }
    glm::vec3*GObject::GetPosAsPtr() { return &this->pos; }
    glm::vec3 GObject::GetRotEuler() { return this->rot; }
    glm::quat GObject::GetRotation() { return glm::quat(this->rot); }
    glm::vec3 GObject::GetScale() { return this->scale; }
    glm::vec3 GObject::GetOrientation() { return this->orient; }
    glm::vec3 GObject::GetOrientWithPos() { return this->orient + this->pos; }
    glm::vec3*GObject::GetOrientAsPtr() { return &this->orient; }
    glm::vec3 GObject::GetForwardDef() { return this->forward; }

    glm::vec3 GObject::GetLocalPos() { return this->localPos; }
    glm::vec3*GObject::GetLocalPosPtr() { return &this->localPos; }
    glm::vec3 GObject::GetLocalRotEuler() { return this->localRot; }
    glm::quat GObject::GetLocalRot() { return glm::quat(this->localRot); }
    glm::vec3 GObject::GetLocalScale() { return this->localScale; }

    glm::vec3 GObject::GetVelocity() { return this->vel; }

    void GObject::UpdateModelMtx() {
        glm::mat4 modelMtx = glm::scale(glm::mat4(1.0), this->scale);
        glm::mat4 rotMtx = glm::toMat4(this->GetRotation()) * modelMtx;
        glm::mat4 posMtx = glm::translate(glm::mat4(1.0), this->pos);
        modelMtx = posMtx * rotMtx * modelMtx;
        this->vao->SetModelMtx(modelMtx);
        // update each child
        for (const auto& child : children) {
            child->localPos = this->pos + glm::vec3(glm::vec4(child->pos,1.0) * modelMtx);
            child->localRot = this->rot;
            child->localScale = this->scale;
            child->orient = this->orient;
            child->UpdateModelMtx(modelMtx);
        }
    }

    void GObject::UpdateModelMtx(glm::mat4 parentModelMtx) {
        glm::mat4 modelMtx = glm::scale(glm::mat4(1.0), this->scale);
        glm::mat4 rotMtx = glm::toMat4(this->GetRotation()) * modelMtx;
        glm::mat4 posMtx = glm::translate(glm::mat4(1.0), this->pos);
        modelMtx = parentModelMtx * posMtx * rotMtx * modelMtx;
        this->vao->SetModelMtx(modelMtx);
    }

    void GObject::Update() {
        this->orient = this->GetRotation() * this->forward;
    }

    EnemyGO::EnemyGO(const Renderer &renderer) : GObject(renderer) {
        GEngine& engine = GEngine::Instance();
        this->player = engine.GetGameObject(this->playerObj);
    }

    glm::vec3 DirToEulerRot(glm::vec3 dir) {
        glm::vec3 ndir = glm::normalize(dir);
        glm::vec3 up  = glm::vec3(0,1,0);
        glm::vec3 hdg = glm::vec3(ndir.x,ndir.z,0);
        double angle_hdg = atan2(ndir.z,ndir.x);
        double angle_pit = asin(ndir.y);
        glm::vec3 w0 = glm::vec3(-ndir.z,ndir.x,0);
        glm::vec3 u0 = glm::cross(w0, ndir);
        double angle_bnk = atan2(glm::dot(w0,up)/glm::length(w0),glm::dot(u0,up)/glm::length(u0));

        return glm::vec3(angle_hdg,angle_pit,angle_bnk);
    }

    void EnemyGO::Update() {
        if (player == nullptr) {
            GEngine& engine = GEngine::Instance();
            this->player = engine.GetGameObject(this->playerObj);
            if (this->player == nullptr) return;
        }
        glm::vec3 playerDir = player->GetPosition() - this->pos;
        this->rot = DirToEulerRot(playerDir);
        GObject::Update();
        this->UpdateModelMtx();
        this->vel = glm::normalize(playerDir) * glm::vec3(30.0);
        this->PlayerCollisionCheck();
    }

    void EnemyGO::PlayerCollisionCheck() {
        glm::vec3 playerPos = this->player->GetPosition();
        if (glm::abs(glm::distance(playerPos, this->pos)) < 1.0) {
            // if the enemy touches the player, "corrupt player" by closing game >:D
            std::exit(1337);
        }
    }

    GoalGO::GoalGO(const Renderer &renderer) : GObject(renderer) {
        GEngine& engine = GEngine::Instance();
        this->player = dynamic_cast<PlayerGO*>(engine.GetGameObject(this->playerObj));
    }

    void GoalGO::Update() {
        GObject::Update();
        this->PlayerCollisionCheck();
    }

    void GoalGO::PlayerCollisionCheck() {
        glm::vec3 playerPos = this->player->GetPosition();
        if (glm::abs(glm::distance(playerPos, this->pos)) < 4.0) {
            // if the goal touches the player, increment player's goal count
            this->player->goalCount++;
            const std::string _name = this->name;
            // then remove the goal from existence
            delete this;
        }
    }

    PlayerGO::PlayerGO(const Renderer &renderer) : GObject(renderer) {}

    void PlayerGO::Update() {
        if (this->goalCount >= goalsToWin) {
            printf("You win!");
            std::exit(0);
        }
        GObject::Update();
    }
}
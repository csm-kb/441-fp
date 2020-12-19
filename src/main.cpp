#define SDL_MAIN_HANDLED

#include <SDL2/SDL.h>
#include <glm/gtx/string_cast.hpp>
#include "GEngine.h"
#include <iostream>

using namespace kVox;

int MOUSE_X_OLD, MOUSE_Y_OLD = 0;

int game_init(GEngine& engine, Renderer& renderer);

bool setupShaders(Renderer& renderer) {
    auto* mShader = new Shader("assets/shaders/simple.v.glsl", "assets/shaders/simple.f.glsl");
    if (!mShader->IsGood()) { return false; }
    renderer.AddShader("simple", mShader);

    mShader = new Shader("assets/shaders/flatShader.v.glsl", "assets/shaders/flatShader.f.glsl");
    if (!mShader->IsGood()) { return false; }
    renderer.AddShader("flat", mShader);

    return true;
}

int main(int argc, char *argv[]) {

    GEngine& engine = GEngine::Instance();

    bool init = engine.Init();
    if (init) {
        Renderer& renderer = engine.GetRenderer();
        // setup simple shaders and drawables
//        if (!setupShaders(renderer)) { engine.Shutdown(); return 1; }
        // init the game-side mechanics, etc
        if (!game_init(engine, renderer)) {
            // if successful, begin game loop!
            engine.Run();
        }
    }

    engine.Shutdown();

    return 0;
}

////////////////////////////

void setupCameras(Renderer& renderer);
bool setupGObjects(GEngine& engine, Renderer& renderer) {
//    float tri_verts[] = {
//            0.0f,  0.5f,  0.0f,     // top
//            0.5f, -0.5f,  0.0f,     // right
//            -0.5f, -0.5f,  0.0f     // left
//    };
//    // test triangle
//    renderer.AddDrawable(new VAO(tri_verts, 9, "simple", renderer));

    // add a light gray torus as the spaceship base and player object
    auto* torus = new PlayerGO(renderer);
    engine.AddGameObject("torus", torus);
    VAO* vao = new PrimitiveVAO(nullptr, 0, "lighting", renderer, PrimitiveType::TORUS);
    vao->material.materialAmbColor = glm::vec3(0.1);
    vao->material.materialDiffColor = glm::vec3(0.9);
    vao->material.materialSpecColor = glm::vec3(1.0);
    vao->material.materialShininess = 0.4;
    torus->SetVAO(vao);
    torus->UpdateModelMtx();
    torus->EnablePhys(); // enable physics for the torus
    renderer.AddDrawable(vao);

    // add a cone pointing forwards to the spaceship
    auto* cone = new GObject(renderer);
    engine.AddGameObject("torus_cone", cone);
    // cone is child of spaceship
    cone->parent = torus;
    torus->children.insert(cone);
    vao = new PrimitiveVAO(nullptr, 0, "lighting", renderer, PrimitiveType::CONE);
    vao->material.materialAmbColor = glm::vec3(0.1);
    vao->material.materialDiffColor = glm::vec3(0.9, 0.7, 0.9);
    vao->material.materialSpecColor = glm::vec3(1.0);
    vao->material.materialShininess = 0.4;
    cone->SetVAO(vao);
    cone->SetRotation(glm::radians(90.0),0.0,0.0,false);
    cone->SetPosition(0.0,0.0,1.0,false);
    cone->UpdateModelMtx();
    renderer.AddDrawable(vao);

    // add a smaller cube attached to the spaceship's rear
    auto* cube = new GObject(renderer);
    engine.AddGameObject("torus_cube", cube);
    // cube is child of spaceship
    cube->parent = torus;
    torus->children.insert(cube);
    vao = new PrimitiveVAO(nullptr, 0, "lighting", renderer, PrimitiveType::CUBE);
    vao->material.materialAmbColor = glm::vec3(0.1);
    vao->material.materialDiffColor = glm::vec3(0.9, 0.7, 0.9);
    vao->material.materialSpecColor = glm::vec3(1.0);
    vao->material.materialShininess = 0.4;
    cube->SetVAO(vao);
    cube->SetRotation(glm::radians(90.0),0.0,0.0,false);
    cube->SetPosition(0.0,0.0,-1.0,false);
    cube->UpdateModelMtx();
    renderer.AddDrawable(vao);

    // add an invisible object for camera anchoring
    auto* cameraAnchor = new GObject(renderer);
    engine.AddGameObject("torus_cam", cameraAnchor);
    cameraAnchor->parent = torus;
    torus->children.insert(cameraAnchor);
    vao = new PrimitiveVAO(nullptr, 0, "lighting", renderer, PrimitiveType::CUBE);
    vao->material.materialAmbColor = glm::vec3(0.1);
    vao->material.materialDiffColor = glm::vec3(0.9, 0.7, 0.9);
    vao->material.materialSpecColor = glm::vec3(1.0);
    vao->material.materialShininess = 0.4;
    cameraAnchor->SetVAO(vao);
    cameraAnchor->SetPosition(0.0,0.0,3.0,false);
    cameraAnchor->SetScale(glm::vec3(0)); // invisible
    cameraAnchor->UpdateModelMtx();
    renderer.AddDrawable(vao);

    // cube rotates based on how fast the spaceship is going
    // also, things will jitter more the faster you go...
    auto* cubeAnim = new AnimHandler_t([](double interp) -> void {
        GEngine& engine = GEngine::Instance();
        Renderer& renderer = engine.GetRenderer();
        double rotMaxVel = glm::radians(90.0); // 90 deg/s
        GObject* cube = engine.GetGameObject("torus_cube");
        double _interp = glm::clamp(glm::length(cube->parent->GetVelocity()) / 1000.0, 0.0, 1.0);
        renderer.UpdateShaderFloat("lighting","jitterStrength",_interp*10.0);
        glm::vec3 rot = cube->GetRotEuler();
        cube->SetRotation(rot.x-(rotMaxVel*_interp*0.3),rot.y+(rotMaxVel*_interp),rot.z+(rotMaxVel*_interp*0.3));
        cube->parent->UpdateModelMtx();
    });
    auto cubeAnimHandler = std::make_shared<kStateAnimHandler>(*cubeAnim);
    engine.RegisterAnim(cubeAnimHandler);

    /////////////////////////////////////////////
    // set the main camera to look at this object
    Camera* mainCam = renderer.GetCameraWithName("main");
    mainCam->SetTargetLookAt(torus->GetPosAsPtr());
    mainCam->SetLookingAtTgt(true);
    mainCam->camDist = 4;
    mainCam->RecomputeCamPos();
    // position the other camera correctly
    Camera* ssCam = renderer.GetCameraWithName("ss_front");
    ssCam->SetTargetLookAt(cameraAnchor->GetLocalPosPtr());
    ssCam->orientLocked = true;
    ssCam->orientPos = cameraAnchor->GetOrientAsPtr();
    ssCam->camDist = 0.001;
    ssCam->SetLookingAtTgt(true);
    ssCam->RecomputeCamPos();
    /////////////////////////////////////////////
    // add an enemy ring object that flies towards the player
    auto* enemy1 = new EnemyGO(renderer);
    engine.AddGameObject("enemy_1", enemy1);
    vao = new PrimitiveVAO(nullptr, 0, "lighting", renderer, PrimitiveType::CYLINDER);
    vao->material.materialAmbColor = glm::vec3(31.0/255.0,0.0/255.0,0.0/255.0);
    vao->material.materialDiffColor = glm::vec3(192.0/255.0,3.0/255.0,3.0/255.0);
    vao->material.materialSpecColor = glm::vec3(220.0/255.0,14.0/255.0,14.0/255.0);
    vao->material.materialShininess = 0.3;
    enemy1->SetVAO(vao);
    enemy1->SetScale(glm::vec3(2.0),false);
    enemy1->SetPosition(glm::vec3(600.0,400.0,-1200.0), false);
    enemy1->UpdateModelMtx();
    enemy1->EnablePhys();
    renderer.AddDrawable(vao);

    // add another enemy ring object that flies towards the player
    auto* enemy2 = new EnemyGO(renderer);
    engine.AddGameObject("enemy_2", enemy2);
    vao = new PrimitiveVAO(nullptr, 0, "lighting", renderer, PrimitiveType::CYLINDER);
    vao->material.materialAmbColor = glm::vec3(31.0/255.0,0.0/255.0,0.0/255.0);
    vao->material.materialDiffColor = glm::vec3(192.0/255.0,3.0/255.0,3.0/255.0);
    vao->material.materialSpecColor = glm::vec3(220.0/255.0,14.0/255.0,14.0/255.0);
    vao->material.materialShininess = 0.3;
    enemy2->SetVAO(vao);
    enemy2->SetScale(glm::vec3(2.0),false);
    enemy2->SetPosition(glm::vec3(400.0,-400.0,-700.0), false);
    enemy2->UpdateModelMtx();
    enemy2->EnablePhys();
    renderer.AddDrawable(vao);

    // add a goal ring in the center of the giant red gas giant
    auto* goal1 = new GoalGO(renderer);
    engine.AddGameObject("goal_1", goal1);
    vao = new PrimitiveVAO(nullptr, 0, "lighting", renderer, PrimitiveType::CYLINDER);
    vao->material.materialAmbColor = glm::vec3(0.0/255.0,18.0/255.0,18.0/255.0);
    vao->material.materialDiffColor = glm::vec3(3.0/255.0,192.0/255.0,192.0/255.0);
    vao->material.materialSpecColor = glm::vec3(14.0/255.0,220.0/255.0,220.0/255.0);
    vao->material.materialShininess = 0.3;
    goal1->SetVAO(vao);
    goal1->SetScale(glm::vec3(2.0),false);
    goal1->SetPosition(glm::vec3(600.0,0.0,0.0), false);
    goal1->UpdateModelMtx();
    renderer.AddDrawable(vao);
    // add a goal ring in the center of the teal gas planet
    auto* goal2 = new GoalGO(renderer);
    engine.AddGameObject("goal_2", goal2);
    vao = new PrimitiveVAO(nullptr, 0, "lighting", renderer, PrimitiveType::CYLINDER);
    vao->material.materialAmbColor = glm::vec3(0.0/255.0,18.0/255.0,18.0/255.0);
    vao->material.materialDiffColor = glm::vec3(3.0/255.0,192.0/255.0,192.0/255.0);
    vao->material.materialSpecColor = glm::vec3(14.0/255.0,220.0/255.0,220.0/255.0);
    vao->material.materialShininess = 0.3;
    goal2->SetVAO(vao);
    goal2->SetScale(glm::vec3(2.0),false);
    goal2->SetPosition(glm::vec3(400.0,-400.0,-700.0), false);
    goal2->UpdateModelMtx();
    renderer.AddDrawable(vao);
    // add a goal ring in the center of the hollow cylinder
    auto* goal3 = new GoalGO(renderer);
    engine.AddGameObject("goal_3", goal3);
    vao = new PrimitiveVAO(nullptr, 0, "lighting", renderer, PrimitiveType::CYLINDER);
    vao->material.materialAmbColor = glm::vec3(0.0/255.0,18.0/255.0,18.0/255.0);
    vao->material.materialDiffColor = glm::vec3(3.0/255.0,192.0/255.0,192.0/255.0);
    vao->material.materialSpecColor = glm::vec3(14.0/255.0,220.0/255.0,220.0/255.0);
    vao->material.materialShininess = 0.3;
    goal3->SetVAO(vao);
    goal3->SetScale(glm::vec3(2.0),false);
    goal3->SetPosition(glm::vec3(600.0,400.0,-1200.0), false);
    goal3->UpdateModelMtx();
    renderer.AddDrawable(vao);
    // goal rings rotate
    auto* goalAnim = new AnimHandler_t([](double tDelta) -> void {
        GEngine& engine = GEngine::Instance();
        Renderer& renderer = engine.GetRenderer();
        double rotVel = glm::radians(90.0); // 90 deg/s
        for (int i = 1; i < 4; i++) {
            GObject *goal = engine.GetGameObject("goal_"+std::to_string(i));
            if (goal == nullptr) continue;
            goal->SetRotation(0.0, 0.0, goal->GetRotEuler().z + (rotVel * tDelta));
            goal->UpdateModelMtx();
        }
    });
    auto goalAnimHandler = std::make_shared<kAnimHandler>(*goalAnim);
    engine.RegisterAnim(goalAnimHandler);

    /////////////////////////////////////////////
    // add a planet using a scaled sphere
    auto* planet1 = new GObject(renderer);
    engine.AddGameObject("scarlet", planet1);
    vao = new PrimitiveVAO(nullptr, 0, "lighting", renderer, PrimitiveType::SPHERE);
    vao->material.materialAmbColor = glm::vec3(31.0/255.0,12.0/255.0,15.0/255.0);
    vao->material.materialDiffColor = glm::vec3(227.0/255.0,89.0/255.0,112.0/255.0);
    vao->material.materialSpecColor = glm::vec3(1.0,230.0/255.0,234.0/255.0);
    vao->material.materialShininess = 0.2;
    planet1->SetVAO(vao);
    planet1->SetScale(glm::vec3(16.0),false);
    planet1->SetPosition(glm::vec3(600.0,0.0,0.0),false);
    planet1->UpdateModelMtx();
    renderer.AddDrawable(vao);

    // add another planet using a scaled sphere
    auto* planet2 = new GObject(renderer);
    engine.AddGameObject("teal", planet2);
    vao = new PrimitiveVAO(nullptr, 0, "lighting", renderer, PrimitiveType::SPHERE);
    vao->material.materialAmbColor = glm::vec3(10.0/255.0,36.0/255.0,30.0/255.0);
    vao->material.materialDiffColor = glm::vec3(217.0/255.0,255.0/255.0,247.0/255.0);
    vao->material.materialSpecColor = glm::vec3(1.0,230.0/255.0,234.0/255.0);
    vao->material.materialShininess = 0.2;
    planet2->SetVAO(vao);
    planet2->SetScale(glm::vec3(8.0),false);
    planet2->SetPosition(glm::vec3(400.0,-400.0,-700.0),false);
    planet2->UpdateModelMtx();
    renderer.AddDrawable(vao);

    // add a small sphere that can represent the star
    auto* star = new GObject(renderer);
    engine.AddGameObject("star", star);
    vao = new PrimitiveVAO(nullptr, 0, "lighting", renderer, PrimitiveType::SPHERE);
    vao->material.materialAmbColor = glm::vec3(1.0);
    vao->material.materialDiffColor = glm::vec3(1.0);
    vao->material.materialSpecColor = glm::vec3(1.0);
    vao->material.materialShininess = 0.001;
    star->SetVAO(vao);
    star->SetScale(glm::vec3(16.0),false);
    star->SetPosition(glm::vec3(20000.0),false);
    star->UpdateModelMtx();
    renderer.AddDrawable(vao);

    // add another planet using a scaled cylinder
    auto* planet3 = new GObject(renderer);
    engine.AddGameObject("purple", planet3);
    vao = new PrimitiveVAO(nullptr, 0, "lighting", renderer, PrimitiveType::CYLINDER);
    vao->material.materialAmbColor = glm::vec3(14.0/255.0,7.0/255.0,30.0/255.0);
    vao->material.materialDiffColor = glm::vec3(129.0/255.0,118.0/255.0,247.0/255.0);
    vao->material.materialSpecColor = glm::vec3(221.0,1.0/255.0,234.0/255.0);
    vao->material.materialShininess = 0.2;
    planet3->SetVAO(vao);
    planet3->SetScale(glm::vec3(9.0),false);
    planet3->SetRotation(30.0, 10.0, 2.0, false);
    planet3->SetPosition(glm::vec3(600.0,400.0,-1200.0),false);
    planet3->UpdateModelMtx();
    renderer.AddDrawable(vao);

    return true;
}

/**
 * The actual game's separated mechanics. We handle setting up the game here!<br>
 * <br>
 * Key idea: let the engine handle the main loops, and handle our own here (using threading if need be).
 */
int game_init(GEngine& engine, Renderer& renderer) {
    printf("------------------------------------\n");
    printf("           Stargazer v0.4           \n");
    printf("------------------------------------\n");
    printf(" ESC : quit the game (likely crash) \n");
    printf("  W  : thrust in forward direction  \n");
    printf("  S  : thrust in reverse direction  \n");
    printf("  A  : yaw spacecraft left          \n");
    printf("  D  : yaw spacecraft right         \n");
    printf("  R  : pitch spacecraft up          \n");
    printf("  F  : pitch spacecraft down        \n");
    printf("SPACE: kill velocity to zero        \n");
    printf("  1  : third-person camera (default)\n");
    printf("  2  : 'first-person' camera        \n");
    printf("------------------------------------\n");
    printf("Welcome to my hand-built spaceship  \n");
    printf("simulator, with a game engine built \n");
    printf("from scratch in the SDL window frame\n");
    printf("-work and OpenGL 4.1! The game has 4\n");
    printf("degrees of freedom and a simple phys\n");
    printf("engine, so feel free to explore a   \n");
    printf("tiny solar system amidst a pretty   \n");
    printf("nebula.                             \n");
    printf("------------------------------------\n");


    // setup cameras (THERE MUST ALWAYS BE AT LEAST ONE)
    setupCameras(renderer);
    // setup game objects
    setupGObjects(engine, renderer);

    /// ** setup inputs ** ///

    // register a default "close the program using the escape key" listener
    KeyInputCallback_t* escCallback = new KeyInputCallback_t([](const bool isPressed, const SDL_KeyboardEvent key) -> void {
        if(isPressed && key.keysym.sym == SDLK_ESCAPE) {
            GEngine::Instance().Shutdown();
        }
    });
    auto quitEscListener = std::make_shared<kKeyInputListener>( *escCallback );
    engine.RegisterKeyInputListener(quitEscListener);

    // register mouse motion listener for arcball-style camera movement
    MouseMotionCallback_t* moveMouseCB = new MouseMotionCallback_t([](const SDL_MouseMotionEvent event) -> void {
        GEngine& engine = GEngine::Instance();
        Renderer& renderer = engine.GetRenderer();
        Camera* camera = renderer.ActiveCamera();
        // functions for arcball movement
        if (engine.IsMouseButtonPressed(SDL_BUTTON_RIGHT) && camera->GetLookingAtTgt() && camera->canLook) {
            camera->camDist += (event.y - MOUSE_Y_OLD) * 0.05;
        }
        if (engine.IsMouseButtonPressed(SDL_BUTTON_LEFT) && camera->GetLookingAtTgt() && camera->canLook) {
            camera->cameraTheta = camera->cameraTheta + ((double)(event.x - MOUSE_X_OLD)*0.005);
            camera->cameraPhi = (camera->cameraPhi >= glm::pi<double>()) ? glm::pi<double>() - 0.001 :
                                (camera->cameraPhi <= 0.0) ? 0.001 : camera->cameraPhi + ((double)(event.y - MOUSE_Y_OLD)*0.005);
        }

        MOUSE_X_OLD = event.x;
        MOUSE_Y_OLD = event.y;
    });
    auto mMotionListener = std::make_shared<kMouseMotionListener>( *moveMouseCB );
    engine.RegisterMouseMotionListener(mMotionListener);

    // register spaceship movement listener for user input
    KeyInputCallback_t* movementCB = new KeyInputCallback_t([](const bool isPressed, const SDL_KeyboardEvent key) -> void {
        GEngine& engine = GEngine::Instance();
        Renderer& renderer = engine.GetRenderer();
        GObject* spaceship = engine.GetGameObject("torus");
        if (spaceship == nullptr) return;

        glm::vec3 orientation = spaceship->GetOrientation();

        switch (key.keysym.sym) {
            case SDLK_w: {
                renderer.SetShake(engine.IsKeyPressed(SDLK_w));
                if (engine.IsKeyPressed(SDLK_w)) {
                    glm::vec3 vel = spaceship->GetVelocity();
                    spaceship->SetVelocity(vel + (orientation * glm::vec3(10.0)));
                }
                break;
            }
            case SDLK_s: {
                renderer.SetShake(engine.IsKeyPressed(SDLK_s));
                if (engine.IsKeyPressed(SDLK_s)) {
                    glm::vec3 vel = spaceship->GetVelocity();
                    spaceship->SetVelocity(vel - (orientation * glm::vec3(10.0)));
                }
                break;
            }
            case SDLK_SPACE: {
                if (engine.IsKeyPressed(SDLK_SPACE)) {
                    spaceship->SetVelocity(glm::vec3(0.0));
                }
                break;
            }
            case SDLK_a: {
                if (engine.IsKeyPressed(SDLK_a)) {
                    glm::vec3 rotA = spaceship->GetRotEuler();
                    rotA += glm::vec3(0.0, 0.25, 0.0);
                    spaceship->SetRotation(rotA,false);
                }
                break;
            }
            case SDLK_d: {
                if (engine.IsKeyPressed(SDLK_d)) {
                    glm::vec3 rotA = spaceship->GetRotEuler();
                    rotA += glm::vec3(0.0, -0.25, 0.0);
                    spaceship->SetRotation(rotA,false);
                }
                break;
            }
            case SDLK_r: {
                if (engine.IsKeyPressed(SDLK_r)) {
                    glm::vec3 rotA = spaceship->GetRotEuler();
                    rotA += glm::vec3(-0.25, 0.0, 0.0);
                    spaceship->SetRotation(rotA,false);
                }
                break;
            }
            case SDLK_f: {
                if (engine.IsKeyPressed(SDLK_f)) {
                    glm::vec3 rotA = spaceship->GetRotEuler();
                    rotA += glm::vec3(0.25, 0.0, 0.0);
                    spaceship->SetRotation(rotA,false);
                }
                break;
            }
            default: {
                break;
            }
        }
    });
    auto moveKeyListener = std::make_shared<kKeyInputListener>( *movementCB );
    engine.RegisterKeyInputListener(moveKeyListener);

    // register camera swap buttons listener
    KeyInputCallback_t* camSwapCB = new KeyInputCallback_t([](const bool isPressed, const SDL_KeyboardEvent key) -> void {
        GEngine& engine = GEngine::Instance();
        Renderer& renderer = engine.GetRenderer();
        switch (key.keysym.sym) {
            case SDLK_1: {
                if (engine.IsKeyPressed(SDLK_1)) {
                    renderer.SetActiveCamera("main");
                }
                break;
            }
            case SDLK_2: {
                if (engine.IsKeyPressed(SDLK_2)) {
                    renderer.SetActiveCamera("ss_front");
                }
                break;
            }
            default: {
                break;
            }
        }
    });
    auto camSwapListener = std::make_shared<kKeyInputListener>( *camSwapCB );
    engine.RegisterKeyInputListener(camSwapListener);

    return 0;
}

void setupCameras(Renderer& renderer) {
    // setup a main camera
    auto* camera = new Camera();
    camera->camPos = glm::vec3(-1,0,0);
    camera->cameraTheta = glm::pi<double>() / 2.8;
    camera->cameraPhi = 0.02;
    camera->camDist = 5.0;
    camera->canLook = true;
    camera->SetTargetLookAt(renderer.GetOrigin());
    camera->RecomputeCamPos();
    renderer.AddCamera("main", camera);
    renderer.SetActiveCamera("main");

    // setup a forward-facing camera for the spaceship
    camera = new Camera();
    camera->camPos = glm::vec3(0.0,0.0,1.05);
    camera->cameraTheta = 0.0;
    camera->cameraPhi = glm::pi<double>() / 2.0;
    camera->camDist = 0.005;
    camera->SetTargetLookAt(renderer.GetOrigin());
    camera->RecomputeCamPos();
    renderer.AddCamera("ss_front", camera);
}
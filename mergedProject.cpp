// GLAD & GLFW
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
// Headers(Jeyoung)
#include "headers/shader.h"
#include "headers/camera.h"
#include "headers/vaoutils.h"
#include "headers/scene.h"
#include "headers/objectMesh.h"
#include "headers/anim_model.h"
#include "headers/ourModel.h"
#include "headers/helper.h"
#include "headers/animation.h"
#include "headers/animator.h"
#include "headers/boneTransformCache.h"
#include "headers/openGLDebugDrawer.h"
#include "headers/particleSystem.h"
#include "headers/ourBullets.h"
// ImGUI Headers(Jeyoung)
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imstb_rectpack.h"
#include "imgui/imstb_textedit.h"
#include "imgui/imstb_truetype.h"
// ImGUI Backend Headers(Jeyoung)
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
// Headers(Sanghyeok)
#include <iostream>
#include "model.h"
#include "loader.h"
#include "cubemap.h"
#include "floor.h"
#include "rectangle.h"
#include "circle.h"
#include "column.h"
#include "wall.h"
#include "fence.h"
#include "obstacle.h"
#include "ram.h"
#include "ball.h"

#define DO_NOT_REMOVE 1
#define IS_EXTERN_GPU 1
#define IS_NOT_FRAPS 0

// Namespace
using namespace std;

// Prototypes(WINDOW)
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window, vector<Animation> &defaultAnimations, vector<Animation> &runAnimations);
void updateTPSCamera();
// Prototypes(ANIMATION)
void renderNode(Node *node);
void updateNodeTransformations(Node *node, glm::mat4 transformationThusFar);
// Prototypes(Stone Ball)
void renderSphere();

// Window
bool VSYNC = true;
bool FULLSCREEN = false;
int WINDOW_WIDTH = 1920;
int WINDOW_HEIGHT = 1080;
int FPS = 999999;

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

// Mouse Event
float yaw = -90.0f;
float pitch = 0.0f;
float radius = 5.0f;
float mouseSensitivity = 0.02f;
float lastX = WINDOW_WIDTH / 2.0f;
float lastY = WINDOW_HEIGHT / 2.0f;
bool firstMouse = true;
float fov = 45.0f;

// Frame
float deltaTime = 0.0f;

// Animator & Animation Variable
Animator animator = Animator();
const int defaultIdle = 9;

// Character Transform
Node *character = createSceneNode();
BoneTransformCache boneCache;

// Debug Drawing(Toggle V)
Shader *debugShader = nullptr;
bool debugDrawEnabled = false;

// Physics
const float GRAVITY = -9.8f;
float velocityY = 0.0f;
static bool isInAir = false;
btVector3 nextVelocityXZ = btVector3(0, 0, 0);
bool isOnSlope = false;
bool wasOnSlopeBeforeJump = false;
bool isRolling = false;

// Bullet(Floors)
btDiscreteDynamicsWorld *dynamicsWorld = nullptr;
btRigidBody *groundBody = nullptr;
btRigidBody *groundWallBodies[5];
btRigidBody *slopeBody = nullptr;
btRigidBody *slopeEdgeBodyLeft = nullptr;
btRigidBody *slopeEdgeBodyRight = nullptr;
// Bullet(Objects)
btRigidBody *characterBody = nullptr;
btRigidBody *ballBody = nullptr;
vector<btRigidBody *> obstacleBodies;
vector<btRigidBody *> ramBodies;
btCapsuleShape *normalShape = nullptr;
btCapsuleShape *extendedShape = nullptr;

float gCharacterRadius = 0.0f;
float gCapsuleHeight = 0.0f;
float gTotalColliderHeight = 0.0f;

#if IS_EXTERN_GPU
extern "C"
{
    _declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
    _declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

unsigned int Wall::texture = 0;
unsigned int Wall::normal = 0;
unsigned int Wall::roughness = 0;
unsigned int Wall::ao = 0;
float lane_length = 230.f;
float lane_angle = glm::radians(15.f);

bool isPushingBall = false;

int main()
{
    // ----------------------------------------------------- GLFW START -----------------------------------------------------

    // Init GLFW
    glfwInit();
    // Define OpenGL version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // Define usage of Core
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // For AA
    glfwWindowHint(GLFW_SAMPLES, 4);

    // Create window with GLFW
    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Project", FULLSCREEN ? glfwGetPrimaryMonitor() : NULL, NULL);
    if (window == NULL)
    {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }

    // IMGUI Init
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 430");

    stbi_set_flip_vertically_on_load(true);
    // ----------------------------------------------------- GLFW END -----------------------------------------------------

    // --------------------------------------------------- BULLET START ---------------------------------------------------
    // Bullet Init
    btBroadphaseInterface* broadphase = new btDbvtBroadphase();                                      // Broadphase collision detection algorithm
    btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration(); // Collision configuration
    btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);           // Responsible for actual collision handling
    btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver();         // Calculates physical response after collision

    // dynamicsWorld: The core of the physics engine (sets gravity, manages simulation)
    dynamicsWorld = new btDiscreteDynamicsWorld(
        dispatcher,
        broadphase,
        solver,
        collisionConfiguration);

    // Set Gravity
    dynamicsWorld->setGravity(btVector3(0, GRAVITY, 0));

    // Debug Drawer
    OpenGLDebugDrawer *debugDrawer = new OpenGLDebugDrawer();
    dynamicsWorld->setDebugDrawer(debugDrawer);
    debugDrawer->setDebugMode(
        btIDebugDraw::DBG_DrawWireframe |
        btIDebugDraw::DBG_DrawAabb |
        btIDebugDraw::DBG_DrawContactPoints);

    // Ground & Slope
    float laneLengthX = 20.0f;
    float slopeLengthX = 0.5f * laneLengthX;
    float slopeLengthZ = 0.5f * lane_length;
    float groundSize = 20.0f;
    float wallDepth = 0.3f;
    std::vector<glm::vec3> groundWallPositions = {
        {groundSize + 0.5f * wallDepth, 1.0f, 0.0f},                               // +x
        {-groundSize - 0.5f * wallDepth, 1.0f, 0.0f},                              // -x
        {0.0f, 1.0f, groundSize + 0.5f * wallDepth},                               // +z
        {-groundSize + 0.5f * slopeLengthX, 1.0f, -groundSize - 0.5f * wallDepth}, // -z(-x offset)
        {groundSize - 0.5f * slopeLengthX, 1.0f, -groundSize - 0.5 * wallDepth}    // -z(+x offset)
    };
    std::vector<glm::vec3> groundWallSizes = {
        {0.15f, 1.0f, groundSize},
        {0.15f, 1.0f, groundSize},
        {groundSize, 1.0f, 0.15f},
        {(groundSize - slopeLengthX) * 0.5f, 1.0f, 0.15f},
        {(groundSize - slopeLengthX) * 0.5f, 1.0f, 0.15f}};

    // Add collider of each object
    addGroundToBullet(dynamicsWorld, groundSize);
    for (int idx = 0; idx < 5; idx++)
    {
        addGroundWallToBullet(dynamicsWorld, groundWallPositions[idx], groundWallSizes[idx], idx);
    }

    addSlopeToBullet(dynamicsWorld, slopeLengthX, slopeLengthZ);
    addSlopeLeftEdgeToBullet(dynamicsWorld, slopeLengthX, slopeLengthZ);
    addSlopeRightEdgeToBullet(dynamicsWorld, -slopeLengthX, slopeLengthZ);

    // ---------------------------------------------------- BULLET END ----------------------------------------------------

    // Print log
    printOpenGLInfo();

    // configure global opengl state
    // -----------------------------
    // Depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Disable built-in dithering
    glDisable(GL_DITHER);

    // Enable transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (!VSYNC)
        glfwSwapInterval(0);

    // -------------------------------------------------- ANIMATION START -------------------------------------------------
    // Characters
    string ybotModel = "resources/objects/animations/Y Bot/Model/Y Bot.fbx";
    vector<string> ybotDefaultAnimations = {
        "resources/objects/animations/Y Bot/Default/Walking Forward.fbx",  // 0(W)
        "resources/objects/animations/Y Bot/Default/Walking Left.fbx",     // 1(A)
        "resources/objects/animations/Y Bot/Default/Walking Backward.fbx", // 2(S)
        "resources/objects/animations/Y Bot/Default/Walking Right.fbx",    // 3(D)
        "resources/objects/animations/Y Bot/Default/Walking Push.fbx",     // 4(Walk + Push Stone)
        "resources/objects/animations/Y Bot/Default/Jumping Forward.fbx",  // 5(W + Jump)
        "resources/objects/animations/Y Bot/Default/Jumping Left.fbx",     // 6(A + Jump)
        "resources/objects/animations/Y Bot/Default/Jumping Backward.fbx", // 7(S + Jump)
        "resources/objects/animations/Y Bot/Default/Jumping Right.fbx",    // 8(D + Jump)
        "resources/objects/animations/Y Bot/Default/Idle.fbx",             // 9
    };
    vector<string> ybotRunAnimations = {
        "resources/objects/animations/Y Bot/Run/Running Forward.fbx",  // 0(W)
        "resources/objects/animations/Y Bot/Run/Running Left.fbx",     // 1(A)
        "resources/objects/animations/Y Bot/Run/Running Backward.fbx", // 2(S)
        "resources/objects/animations/Y Bot/Run/Running Right.fbx",    // 3(D)
        "resources/objects/animations/Y Bot/Run/Running Push.fbx",     // 4(Run + Push Stone)
        "resources/objects/animations/Y Bot/Run/Running Jump.fbx",     // 5(WASD + Jump)
    };

    vector<AnimTextureOverride> characterOverrides = {
        {0, DIFFUSE, "resources/textures/Y Bot/Y Bot_DIFFUSE_Baked.png"},
        {0, NORMAL, "resources/textures/Y Bot/Y Bot_Normal_Baked.png"},
        {0, SPECULAR, "resources/textures/Y Bot/Y Bot_Specular_Baked.png"}};

    // Create model object
    AnimModel m = AnimModel(ybotModel, characterOverrides);

    vector<ObjectMesh> squareMeshes = m.meshes;
    //cout << "Loaded meshes: " << m.meshes.size() << endl;

    Node *root = createSceneNode();
    root->type = ROOT;

    vector<Animation> defaultAnimations;
    vector<Animation> runAnimations;
    vector<Animation> crouchAnimations;

    for (int i = 0; i < ybotDefaultAnimations.size(); ++i)
    {
        defaultAnimations.push_back(Animation(ybotDefaultAnimations[i], &m));
    }
    for (int i = 0; i < ybotRunAnimations.size(); ++i)
    {
        runAnimations.push_back(Animation(ybotRunAnimations[i], &m));
    }
    // -------------------------------------------------- ANIMATION END -------------------------------------------------

    // -------------------------------------------------- CHARACTER --------------------------------------------------
    character->type = CHARACTER;
    character->scale = glm::vec3(0.01f, 0.01f, 0.01f);
    // character->scale = glm::vec3(0.1, 0.1, 0.1);
    // character->rotation = glm::vec3(-3.14 / 2.0, 0.0, 0.0);

    for (int i = 0; i < m.meshes.size(); i++)
    {
        unsigned int charVAO = generateBuffer(squareMeshes[i]);
        character->vertexArrayObjectIDs.push_back(charVAO);
        character->VAOIndexCounts.push_back(squareMeshes[i].indices.size());

        character->textureIDs.push_back(m.diffuseMaps[i]);
        character->normalMapIDs.push_back(m.normalMaps[i]);
        character->specularMapIDs.push_back(m.specularMaps[i]);
    }

    addChild(root, character);

    // build and compile shaders
    // -------------------------
    Shader animShader("shaders/animation.vs", "shaders/animation.fs");
    Shader depthShader("shaders/depth.vs", "shaders/depth.fs");
    debugShader = new Shader("shaders/debug.vs", "shaders/debug.fs");
    Shader objectShader("shaders/object.vs", "shaders/object.fs");
    Shader outlineShader("shaders/outline.vs", "shaders/outline.fs");
    Shader dustShader("shaders/particle.vs", "shaders/particle.fs");

    boneCache.build(animShader.ID);
    boneCache.build(depthShader.ID);

    bool wasInAir = false;

    // Render loop
    float frameTime = 1.0f / FPS;
    float lastFrame = 0.0f;

    // Generate Depth Map
    unsigned int depthMap;
    unsigned int depthFBO;
    const unsigned int s_width = 2048, s_height = 2048;
    generateDepthMap(depthMap, depthFBO, s_width, s_height);

    // Start
    cout << "Starting.." << endl;

    // ----------------------------------------------- OBJECTS POSITION -----------------------------------------------
    // Ball
    glm::vec3 slopeCenter = glm::vec3(0.0f, 2.488f + 0.1f, -9.659f);
    const glm::vec3 initialBallPosition = slopeCenter;

    // ----------------------------------------------- OBJECTS TEXTURE -----------------------------------------------
    // Y Bot
    OurModel ybotModelSizeOnly(
        "resources/objects/animations/Y Bot/Model/Y Bot.fbx",
        "resources/textures/Y Bot/Y Bot_DIFFUSE_Baked.png",
        "resources/textures/Y Bot/Y Bot_Normal_Baked.png",
        "resources/textures/Y Bot/Y Bot_Specular_Baked.png");
    
    Ball ballModel("resources/Rock022_1K-JPG/Rock022_1K-JPG_Color.jpg",
                   "resources/Rock022_1K-JPG/Rock022_1K-JPG_NormalGL.jpg",
                   "resources/Rock022_1K-JPG/Rock022_1K-JPG_Roughness.jpg",
                   "resources/Rock022_1K-JPG/Rock022_1K-JPG_AmbientOcclusion.jpg",
                    "resources/Rock022_1K-JPG/Rock022_1K-JPG_Displacement.jpg",
                   renderSphere);

    // 1. Add collider: "Ball"
    glm::vec3 ballScale = glm::vec3(0.9f);
    //glm::vec3 ballModelSize = ballModel.getAABBSize();   // Original size
    glm::vec3 ballModelSize(2.5f);
    glm::vec3 ballWorldSize = ballModelSize * ballScale; // World size
    glm::vec3 ballHalfExtent = ballWorldSize * 0.5f;
    addSphereToBullet(dynamicsWorld, initialBallPosition, ballHalfExtent);
    // 2. Add collider: "Character"
    glm::vec3 characterModelSize = ybotModelSizeOnly.getAABBSize();       // Original size
    glm::vec3 characterWorldSize = characterModelSize * character->scale; // World size
    glm::vec3 characterHalfExtents = characterWorldSize * 0.5f;
    addCharacterToBullet(dynamicsWorld, characterWorldSize, characterHalfExtents, character);

    // ----------------------------------------------------- VAOs & VBOs -----------------------------------------------------
    // Floor
    // -----
    float frustum_far = 300.0f;

    // Cubemap
    // -------
    std::vector<std::string> faces{
        "resources/textures/skybox/right.jpg",
        "resources/textures/skybox/left.jpg",
        "resources/textures/skybox/top.jpg",
        "resources/textures/skybox/bottom.jpg",
        "resources/textures/skybox/front.jpg",
        "resources/textures/skybox/back.jpg"};
    Cubemap cubemap(faces, "shaders/cubemap.vs", "shaders/cubemap.fs");

    // Lanes
    // -----
    glm::vec3 lanePosition = glm::vec3(0.f, 0.f, -20.f);
    Rectangle lane(lanePosition, laneLengthX, lane_length,
                   "resources/textures/Ground074_1K-JPG/Ground074_1K-JPG_Color.jpg",
                   "resources/textures/Ground074_1K-JPG/Ground074_1K-JPG_NormalGL.jpg",
                   "resources/textures/Ground074_1K-JPG/Ground074_1K-JPG_Roughness.jpg",
                   "resources/textures/Ground074_1K-JPG/Ground074_1K-JPG_AmbientOcclusion.jpg",
                   lane_angle,
                   NULL);
    Shader laneShader("shaders/lane.vs", "shaders/lane.fs");
    Rectangle::setShader(laneShader);

    // StartPlace
    // -----
    Rectangle startPlace(glm::vec3(0.f, 0.f, 20.f), 40.f, 40.f,
                         "resources/textures/Ground074_1K-JPG/Ground074_1K-JPG_Color.jpg",
                         "resources/textures/Ground074_1K-JPG/Ground074_1K-JPG_NormalGL.jpg",
                         "resources/textures/Ground074_1K-JPG/Ground074_1K-JPG_Roughness.jpg",
                         "resources/textures/Ground074_1K-JPG/Ground074_1K-JPG_AmbientOcclusion.jpg",
                         glm::radians(0.f),
                         NULL);

    Shader startPlaceShader("shaders/startPlace.vs", "shaders/startPlace.fs");
    startPlace.setShader(startPlaceShader);

    // EndPlace
    // ------
    float endPlace_z = -20.f - lane_length * glm::cos(lane_angle);
    float endPlace_y = lane_length * glm::sin(lane_angle);
    Rectangle endPlace(glm::vec3(0.f, endPlace_y, endPlace_z), 40.f, 40.f,
                       "resources/textures/Ground074_1K-JPG/Ground074_1K-JPG_Color.jpg",
                       "resources/textures/Ground074_1K-JPG/Ground074_1K-JPG_NormalGL.jpg",
                       "resources/textures/Ground074_1K-JPG/Ground074_1K-JPG_Roughness.jpg",
                       "resources/textures/Ground074_1K-JPG/Ground074_1K-JPG_AmbientOcclusion.jpg",
                       glm::radians(0.f),
                       NULL);

    // Column
    // ------
    Shader columnShader("shaders/column.vs", "shaders/column.fs");

    Column column("resources/3dmodels/roman_column2/scene.gltf", 8, glm::radians(150.f), 10.f);

    // Circle
    // ------
    Circle circle(glm::vec3(0.f, 0.15f, 0.f), 32, 5.f,
                  "resources/textures/Plastic013A_1K-JPG/Plastic013A_1K-JPG_Color.jpg");
    Shader circleShader("shaders/circle.vs", "shaders/circle.fs");
    Circle::setShader(circleShader);

    // Wall
    // ------
    Wall wall1(NULL,
               glm::vec3(-1.f, 0.f, 0.f),
               glm::vec3(0.f, 0.f, -1.f),
               0.3f, 2.f, 40.f,
               {glm::vec4(glm::vec3(0.f, 0.f, 20.f), glm::radians(180.f))});

    Wall wall2(NULL,
               glm::vec3(-1.f, 0.f, 0.f),
               glm::vec3(0.f, 0.f, -1.f),
               0.3f, 2.f, 10.f,
               {glm::vec4(glm::vec3(-15.f, 0.f, -20.f), 0.f),
                glm::vec4(glm::vec3(15.f, 0.f, -20.f), 0.f)});

    Wall wall3(NULL,
               glm::vec3(-1.f, 0.f, 0.f),
               glm::vec3(0.f, 0.f, -1.f),
               0.3f, 2.f, 40.f,
               {glm::vec4(glm::vec3(0.f, endPlace_y, endPlace_z - 40.f), 0.f)});

    Wall wall4(NULL,
               glm::vec3(-1.f, 0.f, 0.f),
               glm::vec3(0.f, 0.f, -1.f),
               0.3f, 2.f, 10.f,
               {glm::vec4(glm::vec3(-15.f, endPlace_y, endPlace_z), 0.f),
                glm::vec4(glm::vec3(15.f, endPlace_y, endPlace_z), 0.f)});

    Shader wallShader("shaders/wall.vs", "shaders/wall.fs");
    Wall::setShader(wallShader);
    Wall::loadTextures(
        "resources/PavingStones129_1K-JPG/PavingStones129_1K-JPG_Color.jpg",
        "resources/PavingStones129_1K-JPG/PavingStones129_1K-JPG_NormalGL.jpg",
        "resources/PavingStones129_1K-JPG/PavingStones129_1K-JPG_Roughness.jpg",
        "resources/PavingStones129_1K-JPG/PavingStones129_1K-JPG_AmbientOcclusion.jpg");

    // Fence
    // ------
    Shader fenceShader("shaders/fence.vs", "shaders/fence.fs");

    Fence fence("resources/3dmodels/wooden_fence/scene.gltf",
                lane.model,
                {glm::vec4(-4.5f, 0.f, -140.f, 1),
                 glm::vec4(-4.5f, 0.f, -160.f, 1),
                 glm::vec4(-4.5f, 0.f, -220.f, 1),
                 glm::vec4(4.5f, 0.f, -150.f, 0),
                 glm::vec4(0.f, 0.f, -200.f, 0),
                 glm::vec4(4.5f, 0.f, -220.f, 0)});

    // Obstacle
    // ------
    vector<glm::vec3> obsPositions =
        {
            glm::vec3(3.f, 0.f, -30.f),
            glm::vec3(-5.f, 0.f, -35.f),
            glm::vec3(7.f, 0.f, -40.f),
            glm::vec3(-8.f, 0.f, -45.f),

            glm::vec3(0.7f, 0.f, -55.f),
            glm::vec3(-0.3f, 0.f, -57.f),
            glm::vec3(-1.3f, 0.f, -59.f),
            glm::vec3(-2.3f, 0.f, -61.f),
            glm::vec3(-3.3f, 0.f, -63.f),
            glm::vec3(-4.3f, 0.f, -65.f),
            glm::vec3(-5.3f, 0.f, -67.f),
            glm::vec3(-6.3f, 0.f, -69.f),
            glm::vec3(-7.3f, 0.f, -71.f),
            glm::vec3(-8.3f, 0.f, -73.f),
            glm::vec3(-9.3f, 0.f, -75.f),

            glm::vec3(1.3f, 0.f, -75.f),
            glm::vec3(2.3f, 0.f, -77.f),
            glm::vec3(3.3f, 0.f, -79.f),
            glm::vec3(4.3f, 0.f, -81.f),
            glm::vec3(5.3f, 0.f, -83.f),
            glm::vec3(6.3f, 0.f, -85.f),
            glm::vec3(7.3f, 0.f, -87.f),
            glm::vec3(8.3f, 0.f, -89.f),
        };
    Shader obstacleShader("shaders/obstacle.vs", "shaders/obstacle.fs");
    Obstacle obs("resources/3dmodels/pack_of_crates/scene.gltf",
                 lane.model,
                 obsPositions);
    vector<glm::vec3> obsWorldPos = obs.getWorldPositions();
    for (int i = 0; i < obsWorldPos.size(); ++i)
    {
        addObstacleToBullet(dynamicsWorld, obsWorldPos[i], obs.objSize * 0.5f, i);
    }

    vector<glm::vec3> ramPositions =
        {
            glm::vec3(-5.f, 0.f, -95.f),
            glm::vec3(5.f, 0.f, -115.f),
            glm::vec3(-7.f, 0.f, -130.f),
            glm::vec3(7.f, 0.f, -130.f),
            glm::vec3(-7.f, 0.f, -180.f),
            glm::vec3(7.f, 0.f, -180.f)};
    // Log
    // ------
    Shader logShader("shaders/log.vs", "shaders/log.fs");
    Ram ram("resources/3dmodels/log/scene.gltf",
            "resources/3dmodels/log_workbench_asset/scene.gltf",
            ramPositions);
    vector<glm::vec3> ramWorldPos = ram.getWorldPositions();
    for (int i = 0; i < ramWorldPos.size(); ++i)
    {
        addRamToBullet(dynamicsWorld, ramWorldPos[i], ram.objSize * 0.5f, i);
    }

    // lights
    // ------
    float amplify = 1.2f;
    int num_lights = 4;
    glm::vec4 lights[] = {
        // Positions
        // glm::normalize(glm::vec4(1.f, 1.f, 1.f, 0.0f)),
        glm::vec4(0.f, 10.f, 0.f, 1.f),
        lane.model * glm::vec4(-6.f, 8.f, -200.f, 1.f),
        lane.model * glm::vec4(7.f, 7.f, -40.f, 1.f),
        lane.model * glm::vec4(6.f, 8.f, -115.f, 1.f),

        // Colors
        glm::vec4(1.f) * amplify,
        glm::vec4(1.0f) * amplify,
        glm::vec4(1.0f) * amplify,
        glm::vec4(1.f) * amplify};

    Shader lightShader("shaders/light.vs", "shaders/light.fs");

    // Uniform Buffer Object
    // ---------------------
    unsigned int ubo, ubo_light, ubo_campos;
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glGenBuffers(1, &ubo_light);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_light);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(lights), &lights, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glGenBuffers(1, &ubo_campos);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_campos);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(camera.Position), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo_light);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, ubo_campos);

    unsigned int uboIndex, uboIndex_light, uboIndex_campos;

    // Lanes
    uboIndex = glGetUniformBlockIndex(laneShader.ID, "Matrices");
    uboIndex_light = glGetUniformBlockIndex(laneShader.ID, "Light");
    uboIndex_campos = glGetUniformBlockIndex(laneShader.ID, "CamPos");
    glUniformBlockBinding(laneShader.ID, uboIndex, 0);
    glUniformBlockBinding(laneShader.ID, uboIndex_light, 1);
    glUniformBlockBinding(laneShader.ID, uboIndex_campos, 2);
    // StartPlace
    uboIndex = glGetUniformBlockIndex(startPlaceShader.ID, "Matrices");
    glUniformBlockBinding(startPlaceShader.ID, uboIndex, 0);
    uboIndex_light = glGetUniformBlockIndex(startPlaceShader.ID, "Light");
    glUniformBlockBinding(startPlaceShader.ID, uboIndex_light, 1);
    uboIndex_campos = glGetUniformBlockIndex(startPlaceShader.ID, "CamPos");
    glUniformBlockBinding(startPlaceShader.ID, uboIndex_campos, 2);

    // Column
    uboIndex = glGetUniformBlockIndex(columnShader.ID, "Matrices");
    glUniformBlockBinding(columnShader.ID, uboIndex, 0);
    uboIndex_light = glGetUniformBlockIndex(columnShader.ID, "Light");
    glUniformBlockBinding(columnShader.ID, uboIndex_light, 1);
    uboIndex_campos = glGetUniformBlockIndex(columnShader.ID, "CamPos");
    glUniformBlockBinding(columnShader.ID, uboIndex_campos, 2);

    // Circle
    uboIndex = glGetUniformBlockIndex(circleShader.ID, "Matrices");
    glUniformBlockBinding(circleShader.ID, uboIndex, 0);
    uboIndex_light = glGetUniformBlockIndex(circleShader.ID, "Light");
    glUniformBlockBinding(circleShader.ID, uboIndex_light, 1);
    uboIndex_campos = glGetUniformBlockIndex(circleShader.ID, "CamPos");
    glUniformBlockBinding(circleShader.ID, uboIndex_campos, 2);

    // Wall
    uboIndex = glGetUniformBlockIndex(wallShader.ID, "Matrices");
    glUniformBlockBinding(wallShader.ID, uboIndex, 0);
    uboIndex_light = glGetUniformBlockIndex(wallShader.ID, "Light");
    glUniformBlockBinding(wallShader.ID, uboIndex_light, 1);
    uboIndex_campos = glGetUniformBlockIndex(wallShader.ID, "CamPos");
    glUniformBlockBinding(wallShader.ID, uboIndex_campos, 2);

    // Fence
    uboIndex = glGetUniformBlockIndex(fenceShader.ID, "Matrices");
    glUniformBlockBinding(fenceShader.ID, uboIndex, 0);
    uboIndex_light = glGetUniformBlockIndex(fenceShader.ID, "Light");
    glUniformBlockBinding(fenceShader.ID, uboIndex_light, 1);
    uboIndex_campos = glGetUniformBlockIndex(fenceShader.ID, "CamPos");
    glUniformBlockBinding(fenceShader.ID, uboIndex_campos, 2);

    // Obstacle
    uboIndex = glGetUniformBlockIndex(obstacleShader.ID, "Matrices");
    glUniformBlockBinding(obstacleShader.ID, uboIndex, 0);
    uboIndex_light = glGetUniformBlockIndex(obstacleShader.ID, "Light");
    glUniformBlockBinding(obstacleShader.ID, uboIndex_light, 1);
    uboIndex_campos = glGetUniformBlockIndex(obstacleShader.ID, "CamPos");
    glUniformBlockBinding(obstacleShader.ID, uboIndex_campos, 2);

    // Log
    uboIndex = glGetUniformBlockIndex(logShader.ID, "Matrices");
    glUniformBlockBinding(logShader.ID, uboIndex, 0);
    uboIndex_light = glGetUniformBlockIndex(logShader.ID, "Light");
    glUniformBlockBinding(logShader.ID, uboIndex_light, 1);
    uboIndex_campos = glGetUniformBlockIndex(logShader.ID, "CamPos");
    glUniformBlockBinding(logShader.ID, uboIndex_campos, 2);

    // Ball
    uboIndex = glGetUniformBlockIndex(objectShader.ID, "Matrices");
    glUniformBlockBinding(objectShader.ID, uboIndex, 0);
    uboIndex_light = glGetUniformBlockIndex(objectShader.ID, "Light");
    glUniformBlockBinding(objectShader.ID, uboIndex_light, 1);
    uboIndex_campos = glGetUniformBlockIndex(objectShader.ID, "CamPos");
    glUniformBlockBinding(objectShader.ID, uboIndex_campos, 2);
    // draw in wireframe
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // shader configuration
    // --------------------

    float fpsTimer = 0.0f;
    int frameCounter = 0;

    // Render Loop
    // Rendering Order: Animations => Other Objects => Skybox
    while (!glfwWindowShouldClose(window))
    {
        // FPS 계산
        float currentFrame = static_cast<float>(glfwGetTime());

        if (!VSYNC && currentFrame - lastFrame <= frameTime)
            continue;

        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
#ifndef IS_NOT_FRAPS
        fpsTimer += deltaTime;
        frameCounter++;

        if (fpsTimer >= 1.0f)
        {
            // std::cout << "FPS: " << (1.0f / deltaTime) << "\t\r" << std::flush;
            fpsTimer = 0.0f;
            frameCounter = 0;
        }
#endif

        // projection
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, frustum_far);
        glm::mat4 view = camera.GetViewMatrix();

        glm::mat4 vp = projection * view;
        glBindBuffer(GL_UNIFORM_BUFFER, ubo);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(vp));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glBindBuffer(GL_UNIFORM_BUFFER, ubo_campos);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(camera.Position), glm::value_ptr(camera.Position));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

#ifdef DO_NOT_REMOVE
        // ------------------------------------------- DON'T REMOVE THIS LOGIC(START) ------------------------------------------
        dynamicsWorld->stepSimulation(deltaTime);
        dynamicsWorld->debugDrawWorld();

        btTransform trans;
        characterBody->getMotionState()->getWorldTransform(trans);
        btVector3 btPos = trans.getOrigin();

        float visualShiftY = gTotalColliderHeight * 0.5f; // 또는 characterWorldSize.y * 0.5f
        float offsetY = 0.05f;
        character->position = glm::vec3(
            btPos.getX(),
            btPos.getY() - visualShiftY + offsetY, // 시각적으로 발바닥이 지면에 닿게 보정
            btPos.getZ());

        // User Input
        processInput(window, defaultAnimations, runAnimations);

        btCollisionShape *shape = characterBody->getCollisionShape();
        if (shape)
        {
            switch (shape->getShapeType())
            {
            case CAPSULE_SHAPE_PROXYTYPE:
            {
                btCapsuleShape *capsule = static_cast<btCapsuleShape *>(shape);

                float charCenterY = btPos.getY();
                float charMinY = charCenterY - 0.5f * gTotalColliderHeight;
                float groundMaxY = 0.1f;
                break;
            }
            case BOX_SHAPE_PROXYTYPE:
                break;
            default:
                std::cout << "[WARNING] 처리되지 않은 콜라이더 타입입니다. 타입: " << shape->getShapeType() << std::endl;
                break;
            }
        }

        // Collision variable
        int numManifolds = dynamicsWorld->getDispatcher()->getNumManifolds();
        bool tempInAir = true;
        bool tempOnSlope = false;
        bool isOnGround = false;
        isOnSlope = false;

        // Detect collisions
        for (int i = 0; i < numManifolds; ++i)
        {
            btPersistentManifold *manifold = dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
            const btCollisionObject *obA = manifold->getBody0();
            const btCollisionObject *obB = manifold->getBody1();

            bool isCharacterA = (obA == characterBody);
            bool isCharacterB = (obB == characterBody);
            bool isGroundA = (obA == groundBody);
            bool isGroundB = (obB == groundBody);
            bool isSlopeA = (obA == slopeBody);
            bool isSlopeB = (obB == slopeBody);

            bool isGroundContact = (isCharacterA && isGroundB) || (isCharacterB && isGroundA);
            bool isSlopeContact = (isCharacterA && isSlopeB) || (isCharacterB && isSlopeA);

            bool isCharacterBallContact =
                (obA == characterBody && obB == ballBody) ||
                (obA == ballBody && obB == characterBody);

            if (isGroundContact || isSlopeContact)
            {
                int numContacts = manifold->getNumContacts();
                for (int j = 0; j < numContacts; ++j)
                {
                    const btManifoldPoint &pt = manifold->getContactPoint(j);

                    if (pt.getDistance() < 0.2f)
                    {
                        float ny = pt.m_normalWorldOnB.getY();
                        if (std::abs(ny) > 0.5f)
                        {
                            tempInAir = false;

                            if (isSlopeContact)
                                tempOnSlope = true;
                        }
                    }
                }
            }

            if (isCharacterBallContact)
                isPushingBall = true;
            else
                isPushingBall = false;
        }

        isInAir = tempInAir;
        isOnSlope = tempOnSlope;

        // Animation Update
        glm::mat4 model = glm::mat4(1.0f);
        animator.updateAnimation(deltaTime);
        updateNodeTransformations(root, model);
        auto transforms = animator.getFinalBoneMatrices();
#endif
        // ------------------------------------------- DON'T REMOVE THIS LOGIC(END) -------------------------------------------

        // ----------------------------------------------- SHADOW -----------------------------------------------
        glCullFace(GL_FRONT);
        float shadowRange = 60.0f;
        model = glm::mat4(1.0f);
        glm::mat4 lightProjection = glm::ortho(
            -shadowRange, shadowRange,
            -shadowRange, shadowRange,
            1.0f, 150.0f);
        glm::vec3 lightOffset = glm::vec3(0.0f, 30.0f, 30.0f); // 위에서 대각선으로
        glm::vec3 lightPos = character->position + lightOffset;
        glm::mat4 lightView = glm::lookAt(
            lightPos,
            character->position,
            glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 lightSpaceMatrix = lightProjection * lightView;

        depthShader.use();
        boneCache.setBoneTransforms(depthShader.ID, transforms);
        depthShader.setMat4("M", model);
        depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        // Green
        glClearColor(0.5f, 1.0f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, s_width, s_height);
        glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        renderNode(root);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glCullFace(GL_BACK);

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        // ---------------- Shadow End ------------

        // --------------------------------------- 1. CHARACTER RENDERING ---------------------------------------
        // draw scene as normal
        animShader.use();
        boneCache.setBoneTransforms(animShader.ID, transforms);

        character->rotation.y = -yaw;

        glm::mat4 animView = camera.GetViewMatrix();        
        animShader.setMat4("V", animView);
        animShader.setMat4("P", projection);
        animShader.setVec3("cameraPos", camera.Position);
        //animShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        glUniform4fv(5, 1, glm::value_ptr(lightSpaceMatrix));
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, depthMap);

        renderNode(root);

        // ------------------------------------- 2. STONE BALL RENDERING -------------------------------------
        btTransform ballTrans;
        ballBody->getMotionState()->getWorldTransform(ballTrans);
        btVector3 pos = ballTrans.getOrigin();
        glm::vec3 ballPosition = glm::vec3(pos.x(), pos.y(), pos.z());

        // if stone ball falls
        if (pos.getY() <= -10.0f)
            resetBallPositionToInitial(dynamicsWorld, initialBallPosition, ballHalfExtent);

        objectShader.use();
        model = glm::scale(glm::translate(glm::mat4(1.0f), ballPosition), 0.75f * ballWorldSize); // S
        view = glm::mat4(camera.GetViewMatrix());
        objectShader.setMat4("M", model);
        objectShader.setMat4("V", view);
        objectShader.setMat4("P", projection);

        glDisable(GL_BLEND);
        ballModel.Draw(objectShader);
        glEnable(GL_BLEND);
        // ------------------------------------------- 3. OBJECTS DRAWING -------------------------------------------
        // ----------------------------------------
        glDisable(GL_CULL_FACE);
        // draw fence
        fence.draw(fenceShader);
        // draw obstacle
        obs.draw(obstacleShader);
        // draw column
        column.draw(columnShader);
        // draw log
        ram.draw(logShader, lane.model);
        // draw circle
        circle.draw(circleShader);
        // draw lane
        lane.draw(laneShader, wallShader);
        startPlace.draw(startPlaceShader, wallShader);
        endPlace.draw(startPlaceShader, wallShader);

        // draw sphere
        lightShader.use();
        for (int i = 0; i < num_lights; i++)
        {
            glm::mat4 model = glm::mat4(1.f);
            model = glm::translate(model, glm::vec3(lights[i]));
            lightShader.setMat4("model", model);
            lightShader.setVec3("lightColor", glm::vec3(lights[i + 4]));
            renderSphere();
        }

        // draw wall
        model = glm::mat4(1.f);
        wall1.draw(wallShader, model);
        wall2.draw(wallShader, model);
        wall3.draw(wallShader, model);
        wall4.draw(wallShader, model);
        // draw cubemap
        cubemap.draw(view, projection);

        glEnable(GL_CULL_FACE);
        // ----------------------------------------

        // --------------------------------------- DEBUG DRAWING MODE ---------------------------------------
        if (debugDrawer && characterBody && debugDrawEnabled)
        {
            glDepthMask(GL_FALSE);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glLineWidth(2.0f);

            debugShader->use();
            debugShader->setMat4("V", view);
            debugShader->setMat4("P", projection);

            // Draw Colliders
            // 1. Character
            drawCapsuleCollider(characterBody, debugDrawer, btVector3(1, 0, 0));
            // 2. Ground
            drawGroundCollider(groundBody, debugDrawer);
            for (int idx = 0; idx < 5; idx++)
            {
                drawGroundWallCollider(groundWallBodies[idx], debugDrawer);
            }
            // 3. Slope
            drawSlopeCollider(slopeBody, debugDrawer);
            drawSlopeEdgeCollider(slopeEdgeBodyLeft, debugDrawer);
            drawSlopeEdgeCollider(slopeEdgeBodyRight, debugDrawer);
            // 4. Obstacle
            for (auto *body : obstacleBodies)
                drawObstacleCollider(body, debugDrawer);
            // 5. Ram
            for (auto *body : ramBodies)
                drawRamCollider(body, debugDrawer);
            // 6. Stone Ball
            drawSphereCollider(ballBody, debugDrawer, btVector3(1, 1, 0)); // 노란색

            debugDrawer->setViewProjection(camera.GetViewMatrix(), projection);
            debugDrawer->flushLines();

            glDepthMask(GL_TRUE);
            glEnable(GL_DEPTH_TEST);
        }

        // --------------------------------------  IMGUI --------------------------------------
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Total Window
        ImGui::SetNextWindowSize(ImVec2(400, 300)); // 너비, 높이
        ImGui::Begin("Controller Panel", nullptr, ImGuiWindowFlags_NoResize);

        ImGui::Text("Performance");
        ImGui::Separator();
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

        ImGui::Spacing();
        ImGui::Text("Lighting");
        ImGui::Separator();
        ImGui::SliderFloat3("Light Position", &lights[0].x, -30.0f, 30.0f);
        ImGui::SliderFloat("Light Intensity", &amplify, 0.1f, 5.0f);

        ImGui::Spacing();
        ImGui::Text("Character Info");
        ImGui::Separator();
        ImGui::Text("Player Pos: (%.2f, %.2f, %.2f)", btPos.getX(), btPos.getY(), btPos.getZ());
        ImGui::Text("Ball Pos:   (%.2f, %.2f, %.2f)", pos.getX(), pos.getY(), pos.getZ());
        ImGui::Text("Pushing Ball: %s", isPushingBall ? "Yes" : "No");

        ImGui::Spacing();
        ImGui::Checkbox("Debug Draw", &debugDrawEnabled);

        ImGui::End();

        // Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    cout << endl
         << "Terminating.."
         << endl;

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Terminate GLFW if we are done rendering
    glfwTerminate();
    return 0;
}

void updateNodeTransformations(Node *node, glm::mat4 transformationThusFar)
{
    glm::mat4 transformationMatrix =
        glm::translate(node->position) *
        glm::translate(node->referencePoint) *
        glm::rotate(node->rotation.y, glm::vec3(0, 1, 0));

    transformationMatrix *=
        glm::rotate(node->rotation.x, glm::vec3(1, 0, 0)) *
        glm::rotate(node->rotation.z, glm::vec3(0, 0, 1)) *
        glm::scale(node->scale) *
        glm::translate(-node->referencePoint);

    node->currentTransformationMatrix = transformationThusFar * transformationMatrix;

    for (Node *child : node->children)
    {
        updateNodeTransformations(child, node->currentTransformationMatrix);
    }
}

void renderNode(Node *node)
{
    glUniform1ui(4, node->type);
    switch (node->type)
    {
    case CHARACTER:
        for (unsigned int i = 0; i < node->VAOIndexCounts.size(); i++)
            if (node->vertexArrayObjectIDs[i] != -1)
            {
                if (node->textureIDs[i] >= 0)
                {
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, node->textureIDs[i]);
                }

                if (node->normalMapIDs[i] >= 0)
                {
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, node->normalMapIDs[i]);
                }

                if (node->specularMapIDs[i] >= 0)
                {
                    glActiveTexture(GL_TEXTURE2);
                    glBindTexture(GL_TEXTURE_2D, node->specularMapIDs[i]);
                }

                glm::mat4 modelMatrix = node->currentTransformationMatrix;

                glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(modelMatrix));

                glBindVertexArray(node->vertexArrayObjectIDs[i]);
                glDrawElements(GL_TRIANGLES, node->VAOIndexCounts[i], GL_UNSIGNED_INT, nullptr);
            }

        break;
    }

    for (Node *child : node->children)
    {
        renderNode(child);
    }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window, vector<Animation>& defaultAnimations, vector<Animation>& runAnimations)
{
    // 상태 변수들
    static bool isRunning = false;
    static bool cPressed = false, vPressed = false;
    static bool usingExtended = false;

    static std::string moveDir = "NONE";

    static bool isJumping = false;
    static float jumpTimer = 0.0f;

    const float NO_EFFECT = 0.0f;
    float defaultSpeed = 2.0f;
    float defaultRunSpeed = 3.0f;
    float onSlopeRunSpeed = 4.0f;
    bool idle = true;

    enum class PivotState
    {
        None,
        Left,
        Right
    };
    static PivotState pivotState = PivotState::None;

    glm::vec3 forward = glm::normalize(glm::vec3(camera.Front.x, 0, camera.Front.z));
    glm::vec3 right = glm::normalize(glm::cross(forward, cameraUp));

    Animation* anim = nullptr;

    // ESC: Exit
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Mouse capture toggle
    if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Collision visualizer
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS && !vPressed)
    {
        vPressed = true;
        debugDrawEnabled = !debugDrawEnabled;
        std::cout << "Collider Visualization: " << (debugDrawEnabled ? "ON" : "OFF") << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_RELEASE)
        vPressed = false;

    // Only jump animation
    if (isJumping)
    {
        jumpTimer += deltaTime;
        if (jumpTimer >= animator.getCurrentAnimationDurationInSeconds())
        {
            isJumping = false;
            jumpTimer = 0.0f;
            if (pivotState == PivotState::None)
            {
                anim = &defaultAnimations[defaultIdle];

                if (animator.getCurrentAnimation() != anim)
                    animator.playAnimation(anim);
            }
        }
    }
    else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS &&
        !isRolling && !isInAir && !isOnSlope && !cPressed &&
        pivotState == PivotState::None)
    {
        // Jump animation
        isJumping = true;
        isInAir = true;
        jumpTimer = 0.0f;
        wasOnSlopeBeforeJump = isOnSlope;

        if (isRunning)
            anim = &runAnimations[5];
        else
        {
            anim = (moveDir == "Forward") ? &defaultAnimations[5] : (moveDir == "Leftside") ? &defaultAnimations[6]
                : (moveDir == "Backward") ? &defaultAnimations[7]
                : (moveDir == "Rightside") ? &defaultAnimations[8]
                : nullptr;
        }

        glm::vec3 jumpDir(0.0f);

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            jumpDir += forward;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            jumpDir -= right;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            jumpDir -= forward;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            jumpDir += right;

        if (anim && animator.getCurrentAnimation() != anim)
        {
            if (moveDir != "NONE")
                animator.playAnimation(anim, false);
            else
                animator.playAnimation(&defaultAnimations[defaultIdle]);
        }

        // PHYSICS
        if (glm::length(jumpDir) > NO_EFFECT)
            jumpDir = glm::normalize(jumpDir);
        else
            jumpDir = glm::vec3(NO_EFFECT); // 제자리 점프

        // Horizontal speed
        float jumpSpeed = 3.0f;
        btVector3 horizontalVelocity(jumpDir.x * jumpSpeed, NO_EFFECT, jumpDir.z * jumpSpeed);
        btVector3 newVelocity = btVector3(horizontalVelocity.x(), NO_EFFECT, horizontalVelocity.z());
        characterBody->setLinearVelocity(newVelocity);

        // Vertical impulse
        btVector3 verticalImpulse;
        if (wasOnSlopeBeforeJump)
            verticalImpulse = btVector3(NO_EFFECT, 10.f, NO_EFFECT); // 낮고 자연스럽게
        else
            verticalImpulse = btVector3(NO_EFFECT, 15.0f, NO_EFFECT); // 평지일 때 높게

        characterBody->activate(true);
        characterBody->applyCentralImpulse(verticalImpulse);
    }

    // Move process
    if (pivotState == PivotState::None)
    {
        isRunning = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
        isRolling = glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;

        // If you press 'R'
        static bool usingExtended = false;

        if (isRolling && !usingExtended)
        {
            switchCharacterColliderXZExpanded_BoxVersion(dynamicsWorld, true);
            usingExtended = true;
        }
        else if (!isRolling && usingExtended)
        {
            switchCharacterColliderXZExpanded_BoxVersion(dynamicsWorld, false);
            usingExtended = false;
        }

        glm::vec3 moveVec(0.0f);

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            moveVec += forward;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            moveVec -= right;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            moveVec -= forward;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            moveVec += right;

        if (glm::length(moveVec) > 0.0f && !isJumping)
        {
            float speed = isRunning ? (isOnSlope ? onSlopeRunSpeed : defaultRunSpeed) : defaultSpeed;

            if (isPushingBall)
            {
                speed = 4.0f;
            }
            btVector3 targetVelocity = btVector3(moveVec.x, 0, moveVec.z).normalized() * speed;

            btVector3 currVelocity = characterBody->getLinearVelocity();
            btVector3 finalVelocity;

            if (isRolling)
            {
                // moving direction translation
                float smoothing = 0.15f;
                btVector3 currXZ(currVelocity.x(), 0, currVelocity.z());
                finalVelocity = currXZ.lerp(targetVelocity, smoothing);
            }
            else
            {
                finalVelocity = targetVelocity;
            }

            characterBody->activate(true);
            characterBody->setLinearVelocity(btVector3(finalVelocity.x(), currVelocity.y(), finalVelocity.z()));

            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                moveDir = "Forward";
            else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                moveDir = "Leftside";
            else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                moveDir = "Backward";
            else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                moveDir = "Rightside";

            if (isRolling && moveDir != "NONE")
                anim = isRunning ? &runAnimations[4] : &defaultAnimations[4];
            else if (isRunning)
                anim = (moveDir == "Forward") ? &runAnimations[0] : (moveDir == "Leftside") ? &runAnimations[1]
                : (moveDir == "Backward") ? &runAnimations[2]
                : (moveDir == "Rightside") ? &runAnimations[3]
                : nullptr;
            else
                anim = (moveDir == "Forward") ? &defaultAnimations[0] : (moveDir == "Leftside") ? &defaultAnimations[1]
                : (moveDir == "Backward") ? &defaultAnimations[2]
                : (moveDir == "Rightside") ? &defaultAnimations[3]
                : nullptr;

            if (isPushingBall && glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                anim = &defaultAnimations[4]; // Walking push
            if (anim && animator.getCurrentAnimation() != anim)
                animator.playAnimation(anim);

            nextVelocityXZ = finalVelocity;
            idle = false;
        }
    }

    // Only idle animation
    if (idle && !isJumping && !isInAir && pivotState == PivotState::None)
    {
        anim = &defaultAnimations[defaultIdle];

        if (animator.getCurrentAnimation() != anim)
            animator.playAnimation(anim);

        // Damping
        btVector3 vel = characterBody->getLinearVelocity();
        float damping = 0.85f;

        vel.setX(vel.getX() * damping);
        vel.setZ(vel.getZ() * damping);

        if (abs(vel.getX()) < 0.01f)
            vel.setX(0);
        if (abs(vel.getZ()) < 0.01f)
            vel.setZ(0);

        characterBody->setLinearVelocity(vel);
    }

    // Update camera
    updateTPSCamera();
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    yaw += xoffset * mouseSensitivity;
    pitch += yoffset * mouseSensitivity;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < 0.0f)
        pitch = 0.0f;
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void updateTPSCamera()
{
    const float CAMERA_DISTANCE = 4.0f;
    const float CAMERA_HEIGHT = 2.5f;
    const float DISTANCE_BEHIND = 6.0f;
    const float HEIGHT_OFFSET = 1.0f;

    glm::vec3 cameraOffset = glm::normalize(glm::vec3(camera.Front.x, 0.0f, camera.Front.z)) * -CAMERA_DISTANCE;
    camera.Position = character->position + glm::vec3(0.0f, 2.0f, 0.0f) + cameraOffset;

    character->rotation.y = atan2(camera.Front.x, camera.Front.z);

    pitch = (pitch > 40.0f) ? 40.0f : ((pitch < -20.0f) ? -20.0f : pitch);

    glm::vec3 target = character->position + glm::vec3(0.0f, CAMERA_HEIGHT, 0.0f);

    glm::vec3 offset;
    offset.x = radius * cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    offset.y = radius * sin(glm::radians(pitch));
    offset.z = radius * sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    camera.Position = target + offset;
    camera.Front = glm::normalize(target - camera.Position);
}

// renders (and builds at first invocation) a sphere
// -------------------------------------------------
unsigned int sphereVAO = 0;
unsigned int indexCount;
void renderSphere()
{
    if (sphereVAO == 0)
    {
        glGenVertexArrays(1, &sphereVAO);

        unsigned int vbo, ebo;
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uv;
        std::vector<glm::vec3> normals;
        std::vector<unsigned int> indices;

        const unsigned int X_SEGMENTS = 64;
        const unsigned int Y_SEGMENTS = 64;
        const float PI = 3.14159265359f;
        for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
        {
            for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
            {
                float xSegment = (float)x / (float)X_SEGMENTS;
                float ySegment = (float)y / (float)Y_SEGMENTS;
                float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                float yPos = std::cos(ySegment * PI);
                float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

                positions.push_back(glm::vec3(xPos, yPos, zPos));
                uv.push_back(glm::vec2(xSegment, ySegment));
                normals.push_back(glm::vec3(xPos, yPos, zPos));
            }
        }

        bool oddRow = false;
        for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
        {
            if (!oddRow) // even rows: y == 0, y == 2; and so on
            {
                for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
                {
                    indices.push_back(y * (X_SEGMENTS + 1) + x);
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                }
            }
            else
            {
                for (int x = X_SEGMENTS; x >= 0; --x)
                {
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                    indices.push_back(y * (X_SEGMENTS + 1) + x);
                }
            }
            oddRow = !oddRow;
        }
        indexCount = static_cast<unsigned int>(indices.size());

        std::vector<float> data;
        for (unsigned int i = 0; i < positions.size(); ++i)
        {
            data.push_back(positions[i].x);
            data.push_back(positions[i].y);
            data.push_back(positions[i].z);
            if (normals.size() > 0)
            {
                data.push_back(normals[i].x);
                data.push_back(normals[i].y);
                data.push_back(normals[i].z);
            }
            if (uv.size() > 0)
            {
                data.push_back(uv[i].x);
                data.push_back(uv[i].y);
            }
        }
        glBindVertexArray(sphereVAO);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
        unsigned int stride = (3 + 2 + 3) * sizeof(float);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void *)(6 * sizeof(float)));
    }

    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
}
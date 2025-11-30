#define GLEW_STATIC

//Operations in GL and Window Creation
#include <GL/glew.h>
#include <GLFW/glfw3.h>

//Graphic Library Maths vectors, matrices, etc.
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

//Importing Textures
#include <SOIL.h>

//Window Creator
#include <Windows.h>

//Assimp Headers
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

//Common Helper Libraries
#include <iostream>
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <cstdlib> // Required for system()
#include <conio.h>   // For _kbhit() and _getch()
#include <windows.h> // For Sleep()
#include <limits>

//Sean Made Headers
#include "ShaderObj.h"
#include "ShaderProgram.h"
#include "Camera.h"
#include "Mesh.h"
#include "Model.h"
#include "Sphere.h"
using namespace std;
#pragma region Funcs
//Call on Camera move
void CameraCallback(GLFWwindow* window, double xpos, double ypos) {
    Camera* cam = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    if (cam) cam->mouse_callback(window, xpos, ypos);
}

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame
float scalingValue = 1;
float angleValue = 0;
float ambientLighting = 1;
glm::vec3 rotationVector = glm::vec3(0,1,0);
glm::vec3 lightPos = glm::vec3(0,1,0);
#pragma region Vertices
// Quad vertices For post Processing
GLfloat quadVertices[] = {
    // positions   // texcoords
    -1.0f,  1.0f,  0.0f, 1.0f, // top-left
     1.0f,  1.0f,  1.0f, 1.0f, // top-right
     1.0f, -1.0f,  1.0f, 0.0f, // bottom-right
    -1.0f, -1.0f,  0.0f, 0.0f  // bottom-left
};
// Indices to draw with less vertices 
GLuint quadIndices[] = {
    0, 1, 2,  // first triangle
    0, 2, 3   // second triangle
};
float groundVertices[] = {
    // positions             // normals        // texcoords
    -5000.0f, 0.0f, -5000.0f,  0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
     5000.0f, 0.0f, -5000.0f,  0.0f, 1.0f, 0.0f, 100.0f, 0.0f,
     5000.0f, 0.0f,  5000.0f,  0.0f, 1.0f, 0.0f, 100.0f, 100.0f,

    -5000.0f, 0.0f, -5000.0f,  0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
     5000.0f, 0.0f,  5000.0f,  0.0f, 1.0f, 0.0f, 100.0f, 100.0f,
    -5000.0f, 0.0f,  5000.0f,  0.0f, 1.0f, 0.0f,   0.0f, 100.0f
};
//Get string for loading model path
string modelPath() {
    //Settings Setup Prior to window open
    int num = -1;
    
    cout << "Pick a model with 1,2,3" << endl;
    cout << "    " << "1: Car" << endl;
    cout << "    " << "2: Tree" << endl;
    cout << "    " << "3: Player" << endl;

    while (!(cin >> num) || num > 3 || num < 1) {
        cout << "Invalid input. Please enter 1,2, or 3: ";
        cin.clear();
        cin.ignore(INT_MAX, '\n');
    }

    switch (num) {
    case 1:
        scalingValue = 1;
        return "Models/Car/Car.obj";
        break;
    case 2:
        return "Models/Tree/dead_tree_rt_1.obj";
        break;
    case 3:
        rotationVector = glm::vec3(1, 0, 0);
        angleValue = -90;
        return "Models/Player/Model.fbx";
        break;
    }
}
void PrintHelp() {
    cout << "Sean's 3D Model Viewer Command List!" << endl;
    cout << "    Scaling: To scale model type 'scale' " << endl;
    cout << "    Rotating: To rotate a model on the x-axis type 'rotationx' " << endl;
    cout << "    Rotating: To rotate a model on the y-axis type 'rotationy' " << endl;
    cout << "    Rotating: To rotate a model on the z-axis type 'rotationz' " << endl;
    cout << "    Ambient Lighting: To change the ambient lighting intenstity type 'alight' " << endl;
    cout << "    Move Light: To move point light: 'movel' " << endl;
    cout << "    Clearing: To clear screen type 'cls' " << endl;
}
#pragma endregion Vertices
int main() {
    // --- Selecting Model ---
    string path = modelPath();

    // --- GLFW Initialization ---
    if (!glfwInit()) { cerr << "Failed to initialize GLFW\n"; return -1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "ShaderProgram Test", nullptr, nullptr);
    if (!window) { cerr << "Failed to create GLFW window\n"; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) { cerr << "Failed to initialize GLEW\n"; return -1; }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);


    // --- Camera ---
    Camera cam(window);
    cam.SetCamera(window);
    glfwSetWindowUserPointer(window, &cam);
    glfwSetCursorPosCallback(window, CameraCallback);

    // --- Shaders ---
    VertexShader sceneVert("GLSLs/sceneVertexSource.glsl", GL_VERTEX_SHADER); FragmentShader sceneFrag("GLSLs/sceneFragmentSource.glsl", GL_FRAGMENT_SHADER);
    ShaderProgram sceneShader(sceneVert, sceneFrag);

    VertexShader screenVert("GLSLs/screenVertexSource.glsl", GL_VERTEX_SHADER); FragmentShader screenFrag("GLSLs/screenFragmentSource.glsl", GL_FRAGMENT_SHADER);
    ShaderProgram screenShader(screenVert, screenFrag);
    screenShader.AddAttributePointer(2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), "position", (void*)0);
    screenShader.AddAttributePointer(2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), "texcoord", (void*)(2 * sizeof(GLfloat)));

    VertexShader gridVert("GLSLs/GridVertex.glsl", GL_VERTEX_SHADER); FragmentShader gridFrag("GLSLs/GridFragment.glsl", GL_FRAGMENT_SHADER);
    ShaderProgram gridShader(gridVert, gridFrag);
    
    VertexShader modelVert("GLSLs/ModelVertex.glsl", GL_VERTEX_SHADER); FragmentShader modelFrag("GLSLs/ModelFragment.glsl", GL_FRAGMENT_SHADER);
    ShaderProgram modelShader(modelVert, modelFrag);
   

    VertexShader lightVert("GLSLs/LightVertex.glsl", GL_VERTEX_SHADER); FragmentShader lightFrag("GLSLs/LightFragment.glsl", GL_FRAGMENT_SHADER);
    ShaderProgram lightShader(lightVert, lightFrag);
    // --- Quad Mesh for Post Processing ---
    Mesh quadMesh = Mesh(quadVertices, 4, 4, quadIndices, 6);
    screenShader.use();
    screenShader.setInt("screenTexture", 0);
    quadMesh.GenerateEboQuads(screenShader);


    // --- Grid Mesh --- 
    Mesh gridMesh = Mesh(groundVertices, 8, 6);
    GLsizei stride = 8 * sizeof(GLfloat);
    VertexAttribute gridPositionAttribute = VertexAttribute(stride, 3, 0);
    gridMesh.AddAttributePointer(gridPositionAttribute);

    VertexAttribute gridNormalAttribute = VertexAttribute(stride, 3, 3);
    gridMesh.AddAttributePointer(gridNormalAttribute);

    VertexAttribute gridTexCoordinateAttribute = VertexAttribute(stride, 3, 6);
    gridMesh.AddAttributePointer(gridTexCoordinateAttribute);

    gridShader.use();
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 20000.0f);
    //Set up Grid Mesh shader values
    gridShader.setMat4("projection", proj);
    gridShader.setFloat("cellSize", 1.0f);           // each grid cell 1 unit wide
    gridShader.setFloat("lineWidth", 0.01f);         // vary between 0.002 - 0.02 for sharpness
    gridShader.setFloat("fadeDistance", 500.0f);     // fade out after 500 units
    gridShader.setVec3("gridColor", 0.15f, 0.15f, 0.15f);
    gridShader.setVec3("bgColor", 0.8f, 0.8f, 0.8f);
    gridMesh.GenerateMesh();

    // --- Loading Test Model ---
    cout << "Loading Model From: " << path;
    modelShader.use();
    Model testModel(path);

    // --- Light Sphere ---
    lightShader.use();
    lightShader.setVec3("color", glm::vec3(1, 1, 1));
    lightShader.setBool("useTexture", false);
    Sphere sphere(0.5f, 12, 12);
    vector<Texture> nullTextVec;
    Mesh lightSphere(sphere.verticesMesh, sphere.indices, nullTextVec);

    // --- Framebuffer for post-processing ---
    GLuint frameBuffer, texColorBuffer, rboDepthStencil;
    glGenFramebuffers(1, &frameBuffer); glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    //Creates a texture to draw onto to affect the whole screen
    glGenTextures(1, &texColorBuffer);
    glBindTexture(GL_TEXTURE_2D, texColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);

    glGenRenderbuffers(1, &rboDepthStencil);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepthStencil);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboDepthStencil);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    screenShader.setInt("texFramebuffer;", texColorBuffer);

    // --- Projection ---
    sceneShader.use(); sceneShader.setMat4("proj", proj);
    gridShader.use(); gridShader.setMat4("projection", proj);

    int curSelector = 0;

    auto startTime = std::chrono::high_resolution_clock::now();
    string input = "";

    cout << "Type: 'help' to print commands!" << endl;
    // ---------------- Main Loop ----------------
    while (!glfwWindowShouldClose(window)) {
        // --- Time Handling ---
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float>(currentTime - startTime).count();
        // --- Handle input ---
        if (_kbhit()) {
            char ch = _getch(); // Read the key without waiting for Enter
            if (ch == '\r') { // Enter key
                cout << endl;
                if (input == "scale") {
                    cout << endl <<"Enter Scaling Value: ";
                    cin >> scalingValue;
                    if (scalingValue <= 0) {
                        scalingValue = 1;
                        cout << "Error ScalingValue <= 0" << endl;
                    }
                    cout << "Scaling Model: " << scalingValue << endl;
                }
                else if (input == "rotationx")
                {
                    cout << "Rotating on the x-axis" << endl << "Enter Rotation Value (degrees): ";
                    cin >> angleValue;
                    rotationVector = glm::vec3(1, 0, 0);
                    cout << "Rotating Model on the x-axis: " << angleValue <<  " degrees" << endl;
                }
                else if (input == "rotationy")
                {
                    cout << "Rotating on the y-axis" << endl << "Enter Rotation Value (degrees): ";
                    cin >> angleValue;
                    rotationVector = glm::vec3(0, 1, 0);
                    cout << "Rotating Model on the y-axis: " << angleValue << " degrees" << endl;
                }
                else if (input == "rotationz")
                {
                    cout << "Rotating on the z-axis" << endl << "Enter Rotation Value (degrees): ";
                    cin >> angleValue;
                    rotationVector = glm::vec3(0, 0, 1);
                    cout << "Rotating Model on the z-axis: " << angleValue << " degrees" << endl;
                }
                else if (input == "alight") {
                    cout << "Enter Light intenstiy Between 0 & 1: " << endl;
                    float val;
                    while (!(cin >> val) || val < 0 || val > 1)
                    {
                        cout << "ENTER VALUE BETWEEN 0 & 1: " << endl;
                        cin.clear();
                        cin.ignore(INT_MAX, '\n');
                    }
                    ambientLighting = val;
                    cout << "Ambient Light Changing to: " << ambientLighting << endl;
                }
                else if (input == "movel") {
                    float transY;
                    float transX;
                    float transZ;

                    cout << "Enter Translation x: " << endl;
                    while (!(cin >> transX)) {
                        cout << "ENTER FLOAT VALUE: " << endl;
                        cin.clear();
                        cin.ignore(INT_MAX, '\n');
                    }
                    cout << "Enter Translation y: " << endl;
                    while (!(cin >> transY)) {
                        cout << "ENTER FLOAT VALUE: " << endl;
                        cin.clear();
                        cin.ignore(INT_MAX, '\n');
                    }                    
                    cout << "Enter Translation z: " << endl;
                    while (!(cin >> transZ)) {
                        cout << "ENTER FLOAT VALUE: " << endl;
                        cin.clear();
                        cin.ignore(INT_MAX, '\n');
                    }
                    lightPos = glm::vec3(transX, transY, transZ);
                    cout << "Moving light up to: " << "(" << transX << ", " <<  transY << ", " << transZ << ")" << endl;
                }
                else if (input == "help") {
                    PrintHelp();
                }
                else if (input == "cls") {
                    system("cls");
                }
                else {
                    system("cls");
                    cout << "Command not found!" << endl;
                }
                input.clear(); // Reset for next input
            }
            else if (ch == '\b') { // Backspace
                if (!input.empty()) {
                    input.pop_back();
                    std::cout << "\b \b"; // Erase last char from console
                }
            }
            else {
                input.push_back(ch);
                std::cout << ch; // Echo character
            }
        
        }
        if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) 
            curSelector = 0;
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
            curSelector = 1;
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
            curSelector = 2;
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
            curSelector = 3;
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
            curSelector = 4;


        cam.CameraUpdate(window, deltaTime, sceneShader.ID);

        // ---------- Render to framebuffer ----------
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
        glEnable(GL_DEPTH_TEST);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --- Draw Grid ---
        glm::mat4 view = cam.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);
        
        // --- Draw Model ---
        modelShader.use();
        glm::mat4 testModelModel = glm::mat4(1.0f);
        testModelModel = glm::scale(testModelModel, glm::vec3(scalingValue));
        testModelModel = glm::rotate(testModelModel, glm::radians(angleValue), rotationVector);

        modelShader.setMat4("model", testModelModel);
        modelShader.setMat4("view", view);
        modelShader.setMat4("projection", proj);
        modelShader.setInt("texture_diffuse1", 0);
        modelShader.setInt("texture_specular1", 1);
        modelShader.setVec3("lightPos", lightPos);

        glm::vec4 _ambientLight = glm::vec4(ambientLighting, ambientLighting, ambientLighting, 1);
        modelShader.setVec4("ambientLight", _ambientLight);

        // Set your solid color
        modelShader.setVec3("aColor", glm::vec3(1, 1, 1)); // Red
        testModel.Draw(modelShader);

        glm::mat4 lightSphereModel = glm::mat4(1.0f);
        lightSphereModel = glm::translate(lightSphereModel, lightPos);
        lightShader.use();
        lightShader.setMat4("model", lightSphereModel);
        lightShader.setMat4("view", view);
        lightShader.setMat4("projection", proj);
        lightShader.setVec3("color", glm::vec3(1, 0, 0));
        lightSphere.DrawMesh(lightShader);

        //Set uniforms
        gridShader.use();
        gridShader.setMat4("view", view);
        gridShader.setMat4("model", model);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(-1.0f, -1.0f);   // negative pushes *toward* the camera

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        Texture nullTexture;
        gridMesh.DrawMesh(gridShader, nullTexture);

            
        glDisable(GL_POLYGON_OFFSET_FILL);

        // ---------- Post-processing / Screen Quad ----------      
        screenShader.use();
        quadMesh.DrawPostProcessing(screenShader, curSelector, texColorBuffer);

        // --- Reset ---
        glfwSwapBuffers(window);
        glfwPollEvents();


    }
    // ---------------- Cleanup ----------------
    glDeleteRenderbuffers(1, &rboDepthStencil);
    glDeleteTextures(1, &texColorBuffer);
    glDeleteFramebuffers(1, &frameBuffer);

    gridMesh.Deletion();
    quadMesh.EBODeletion();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
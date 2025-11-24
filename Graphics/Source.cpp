//GLEW
#define GLEW_STATIC
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

//Sean Made Headers
#include "ShaderObj.h"
#include "ShaderProgram.h"
#include "Camera.h";
#include "Mesh.h"
using namespace std;
#pragma region Funcs
void CameraCallback(GLFWwindow* window, double xpos, double ypos) {
    Camera* cam = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    if (cam) cam->mouse_callback(window, xpos, ypos);
}

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame
#pragma region Vertices
// Quad vertices
GLfloat quadVertices[] = {
    // positions   // texcoords
    -1.0f,  1.0f,  0.0f, 1.0f, // top-left
     1.0f,  1.0f,  1.0f, 1.0f, // top-right
     1.0f, -1.0f,  1.0f, 0.0f, // bottom-right
    -1.0f, -1.0f,  0.0f, 0.0f  // bottom-left
};

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

#pragma endregion Vertices
int main() {
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

    // --- Assimp initialization ---
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile("model.obj",
        aiProcess_Triangulate | aiProcess_FlipUVs);
    if (!scene)
    {
        printf("Assimp error: %s\n", importer.GetErrorString());
    }

    printf("Model loaded!\n");

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

    //Quad Meshes
    Mesh quadMesh = Mesh(quadVertices, 4, 4, quadIndices, 6);
    screenShader.use();
    screenShader.setInt("screenTexture", 0);
    quadMesh.GenerateEboQuads(screenShader);


    //Generate Meshes VAO, VBO, and Attribute pointers
    Mesh gridMesh = Mesh(groundVertices,  8, 6);
    GLsizei stride = 8 * sizeof(GLfloat);
    VertexAttribute gridPositionAttribute = VertexAttribute(stride, 3, 0);
    gridMesh.AddAttributePointer(gridPositionAttribute);
   
    VertexAttribute gridNormalAttribute = VertexAttribute(stride, 3, 3);
    gridMesh.AddAttributePointer(gridNormalAttribute);

    VertexAttribute gridTexCoordinateAttribute = VertexAttribute(stride, 3, 6);
    gridMesh.AddAttributePointer(gridTexCoordinateAttribute);

    gridMesh.GenerateMesh();

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

    // --- Projection ---
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 20000.0f);
    sceneShader.use(); sceneShader.setMat4("proj", proj);
    gridShader.use(); gridShader.setMat4("projection", proj);

    int curSelector = 0;

    // ---------------- Main Loop ----------------
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // --- Handle input ---
        if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) curSelector = 0;
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) curSelector = 1;
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) curSelector = 2;
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) curSelector = 3;
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) curSelector = 4;

        cam.CameraUpdate(window, deltaTime, sceneShader.ID);

        // ---------- Render to framebuffer ----------
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
        glEnable(GL_DEPTH_TEST);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --- Draw Grid ---
        glm::mat4 view = cam.GetViewMatrix();
        gridMesh.Draw(gridShader, view);
        
        // --- Draw Scene ---
        sceneShader.use();
        sceneShader.setMat4("view", view);
        sceneShader.setMat4("model", glm::mat4(1.0f));

        // Draw your scene objects here...

        // ---------- Post-processing / Screen Quad ----------      
        quadMesh.DrawEBO(screenShader, curSelector, texColorBuffer);

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

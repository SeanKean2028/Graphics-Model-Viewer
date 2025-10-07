#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <iostream>
#include <chrono>
#include <SOIL.h>
#include <Windows.h>

//Sean Made Headers
#include "ShaderObj.h"
#include "ShaderProgram.h"
#include "Camera.h";
using namespace std;
#pragma region Funcs
void CameraCallback(GLFWwindow* window, double xpos, double ypos) {
    Camera* cam = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    if (cam) cam->mouse_callback(window, xpos, ypos);
}

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame
#pragma region Shaders

//Handles attributes as they appear in the vertex array, positions, and 3d Transformations
//Model matrix: position of model to real world
//View matrix: position of camera to real world
//Order matters in matrix multiplication! Projection looks at the view matrix which looks at the model matrix

//Handles coloring of pixels using glsl
//sampler2D = texture, samples at certain points based on mix func. which linearly interpolates between two values based on a third value
//Fragment shader commonly used in post processing effects  

#pragma endregion Shaders
#pragma region Vertices
// Quad vertices
GLfloat quadVertices[] = {
    -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f,  1.0f,  1.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f,

     1.0f, -1.0f,  1.0f, 0.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
    -1.0f,  1.0f,  0.0f, 1.0f
};
float groundVertices[] = {
    -5000.0f, 0.0f, -5000.0f,
     5000.0f, 0.0f, -5000.0f,
     5000.0f, 0.0f,  5000.0f,

    -5000.0f, 0.0f, -5000.0f,
     5000.0f, 0.0f,  5000.0f,
    -5000.0f, 0.0f,  5000.0f
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

    VertexShader gridVert("GLSLs/GridVertex.glsl", GL_VERTEX_SHADER); FragmentShader gridFrag("GLSLs/GridFragment.glsl", GL_FRAGMENT_SHADER);
    ShaderProgram gridShader(gridVert, gridFrag);

    // --- VAOs/VBOs ---
    GLuint vaoQuad, vboQuad;
    glGenVertexArrays(1, &vaoQuad); glGenBuffers(1, &vboQuad);

    glBindVertexArray(vaoQuad);
    glBindBuffer(GL_ARRAY_BUFFER, vboQuad);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    screenShader.AddAttributePointer(2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), "position", (void*)0);
    screenShader.AddAttributePointer(2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), "texcoord", (void*)(2 * sizeof(GLfloat)));
    screenShader.SetAttributePointers();
    glBindVertexArray(0);

    GLuint gridVAO, gridVBO;
    glGenVertexArrays(1, &gridVAO); glGenBuffers(1, &gridVBO);

    glBindVertexArray(gridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);

    gridShader.AddAttributePointer(3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), "aPos", (void*)0);
    gridShader.SetAttributePointers();
    glBindVertexArray(0);

    // --- Framebuffer for post-processing ---
    GLuint frameBuffer, texColorBuffer, rboDepthStencil;
    glGenFramebuffers(1, &frameBuffer); glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

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
        gridShader.use();
        glm::mat4 view = cam.GetViewMatrix();
        gridShader.setMat4("view", view);
        gridShader.setMat4("model", glm::mat4(1.0f));

        glBindVertexArray(gridVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // --- Draw Scene ---
        sceneShader.use();
        sceneShader.setMat4("view", view);
        sceneShader.setMat4("model", glm::mat4(1.0f));

        // Draw your scene objects here...

        // ---------- Post-processing / Screen Quad ----------
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);

        screenShader.use();
        screenShader.setInt("selector", curSelector);

        glBindVertexArray(vaoQuad);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texColorBuffer);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ---------------- Cleanup ----------------
    glDeleteRenderbuffers(1, &rboDepthStencil);
    glDeleteTextures(1, &texColorBuffer);
    glDeleteFramebuffers(1, &frameBuffer);

    glDeleteBuffers(1, &vboQuad);
    glDeleteVertexArrays(1, &vaoQuad);

    glDeleteBuffers(1, &gridVBO);
    glDeleteVertexArrays(1, &gridVAO);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
/*
*
1) Headers
    Include headers for OpenGL, GLFW, GLEW, GLM, SOIL, and Windows.
2) Initialization
    Start a timer with std::chrono
    Initialize GLFW and request an OpenGL 3.3 core profile context
        Create a window and make its OpenGL context current
    Initialize GLEW for OpenGL extension management

    Enable depth testing (GL_DEPTH_TEST) and stencil testing (GL_STENCIL_TEST
3) Vertex Data
   Define vertex data for a cube and a floor (position, color, texture coordinates
   Generate VAOs (vertex array object) and bind it.
   Generate VBOs (vertex buffer object), bind it, and upload vertex data.
 (element buffer object), bind it, and upload element indices (not used in drawing).
4) Shaders
    Write and compile our vertex shaders (handles positions, colors, and matrices).
        For scene vs. screen, and from various objects (grid lines) to our cubes shaders
    Write and compile our fragment shaders (mixes two textures and outputs color).
        same as above
    Link shaders into a shader program and check for errors.
    Get attribute locations (position, color, texcoord) and configure them with glVertexAttribPointer.
    Enable vertex attributes in the VAO.
5) Textures
    Load two textures (cat.png, puppy.png) with SOIL, bind them to texture units 0 and 1, and set parameters.
6) Transformations
    Get uniform locations (model, view, proj, overrideColor) and upload view/projection matrices.
7) Main loop: drawing, clearing, updating
    Enter the main render loop:
        Clear color, depth, and stencil buffers.
        Update the model matrix to rotate the cube over time.
        Draw the cube (glDrawArrays with 36 vertices).
        Draw Reflection Quad
        Draw grid lines
        Configure stencil buffer, draw the floor, then draw the cube’s reflection with inverted Z and darker color.
        Swap buffers and poll for input events.
8) Clean Up
    delete shaders, program, buffers, VAO, and destroy the window.
    Terminate GLFW and exit.
*/
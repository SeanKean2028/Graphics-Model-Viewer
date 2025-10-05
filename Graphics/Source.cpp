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
#include "Camera.h";
using namespace std;
#pragma region Funcs
void CameraCallback(GLFWwindow* window, double xpos, double ypos) {
    Camera* cam = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    if (cam) cam->mouse_callback(window, xpos, ypos);
}
//Set pointers to vertex attributes in the shader to be changed and used in rendering the scene
void specifySceneVertexAttributes(GLuint shaderProgram)
{
    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);

    GLint colAttrib = glGetAttribLocation(shaderProgram, "color");
    glEnableVertexAttribArray(colAttrib);
    glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

    GLint texAttrib = glGetAttribLocation(shaderProgram, "texcoord");
    glEnableVertexAttribArray(texAttrib);
    glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
}

// Set pointers to vertex attributes in the shader to be changed and used in rendering the screen
void specifyScreenVertexAttributes(GLuint shaderProgram)
{
    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

    GLint texAttrib = glGetAttribLocation(shaderProgram, "texcoord");
    glEnableVertexAttribArray(texAttrib);
    glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
}

void createShaderProgram(const GLchar* vertSrc, const GLchar* fragSrc, GLuint& vertexShader, GLuint& fragmentShader, GLuint& shaderProgram)
{
    // Create and compile the vertex shader
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertSrc, NULL);
    glCompileShader(vertexShader);

    // Create and compile the fragment shader
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragSrc, NULL);
    glCompileShader(fragmentShader);

    // Link the vertex and fragment shader into a shader program
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glBindFragDataLocation(shaderProgram, 0, "outColor");
    glLinkProgram(shaderProgram);
}

void createShaderProgram(GLuint& gridVertex, GLuint& gridFrag, GLuint& shaderProgram)
{
    // Link the vertex and fragment shader into a shader program
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, gridVertex);
    glAttachShader(shaderProgram, gridFrag);
    glLinkProgram(shaderProgram);
    glBindFragDataLocation(shaderProgram, 0, "outColor");
    glLinkProgram(shaderProgram);
}
float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame
#pragma region Shaders

//Handles attributes as they appear in the vertex array, positions, and 3d Transformations
//Model matrix: position of model to real world
//View matrix: position of camera to real world
//Order matters in matrix multiplication! Projection looks at the view matrix which looks at the model matrix
//Projection matrix: 3D to 2D Razterization
const GLchar* sceneVertexSource = R"glsl(
        #version 330 core
        in vec3 position;
        in vec3 color;
        in vec2 texcoord;
        
        out vec3 Color;
        out vec2 Texcoord;    

        uniform vec3 overrideColor;
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 proj;
        void main(){
            Color = overrideColor * color;
            Texcoord = texcoord;
            gl_Position = proj * view *  model * vec4(position, 1.0);
        }
    )glsl";
//Handles coloring of pixels using glsl
//sampler2D = texture, samples at certain points based on mix func. which linearly interpolates between two values based on a third value
//Fragment shader commonly used in post processing effects  
const GLchar* sceneFragmentSource = R"glsl(
        #version 330 core
        in vec3 Color;
        uniform float time;
        out vec4 outColor;
        uniform int selector;

        uniform sampler2D texKitten;
        uniform sampler2D texPuppy;

        in vec2 Texcoord;
        


        void main() {
            vec4 colKitten = texture(texKitten, Texcoord);
            vec4 colPuppy = texture(texPuppy, Texcoord);
            outColor = vec4(Color, 1.0) * mix(texture(texKitten, Texcoord),texture(texPuppy, Texcoord), 0.5);
        }
    )glsl";
const GLchar* sceneGridLinesFragmentSource = R"glsl(
        #version 330 core
        in vec3 FragPos;
        out vec4 FragColor;

        void main()
        {
            float lineWidth = 0.02; // thickness
            float gridSpacing = 1.0;

            // repeat space
            float x = abs(mod(FragPos.x, gridSpacing));
            float z = abs(mod(FragPos.z, gridSpacing));

            // near the grid line → dark
            if (x < lineWidth || z < lineWidth)
                FragColor = vec4(0.3, 0.3, 0.3, 1.0);
            else
                discard; // or background color
        }
    )glsl";
const GLchar* sceneGridLinesVertexSource = R"glsl(
        #version 330 core
        layout (location = 0) in vec3 aPos;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;

        out vec3 FragPos;

        void main()
        {
            vec4 worldPos = model * vec4(aPos, 1.0);
            FragPos = worldPos.xyz;
            gl_Position = projection * view * worldPos;
        }
    )glsl";
const GLchar* screenFragmentSource = R"glsl(
        #version 330 core
        in vec2 Texcoord;
        out vec4 outColor;
        uniform sampler2D texFramebuffer;
        uniform int selector;
        
        const float blurSizeH = 1.0 / 300.0;
        const float blurSizeV = 1.0 / 200.0;

        void main()
        {
            if (selector == 1) {
                // Inverse Color
                outColor = vec4(1.0) - texture(texFramebuffer, Texcoord);
            } 
            else if (selector == 2) {
                // Greyscale
                vec4 c = texture(texFramebuffer, Texcoord);
                float avg = (c.r + c.g + c.b) / 3.0;
                outColor = vec4(avg, avg, avg, 1.0);
            } 
            else if (selector == 3) {
                // Simple blur
                vec4 sum = vec4(0.0);
                for (int y = -4; y <= 4; ++y) {
                    for (int x = -4; x <= 4; ++x) {
                        sum += texture(texFramebuffer,
                                       Texcoord + vec2(float(x) * blurSizeH,
                                                       float(y) * blurSizeV));
                    }
                }
                outColor = sum / (9.0 * 9.0);
            } 
            else if (selector == 4) {
                // Edge detection (Sobel-like)
                vec4 top = texture(texFramebuffer, Texcoord + vec2(0.0,  blurSizeV));
                vec4 bottom = texture(texFramebuffer, Texcoord + vec2(0.0, -blurSizeV));
                vec4 left = texture(texFramebuffer, Texcoord + vec2(-blurSizeH, 0.0));
                vec4 right = texture(texFramebuffer, Texcoord + vec2( blurSizeH, 0.0));
                vec4 topLeft = texture(texFramebuffer, Texcoord + vec2(-blurSizeH,  blurSizeV));
                vec4 topRight = texture(texFramebuffer, Texcoord + vec2( blurSizeH,  blurSizeV));
                vec4 bottomLeft = texture(texFramebuffer, Texcoord + vec2(-blurSizeH, -blurSizeV));
                vec4 bottomRight = texture(texFramebuffer, Texcoord + vec2( blurSizeH, -blurSizeV));
                vec4 sx = -topLeft - 2.0 * left - bottomLeft + topRight + 2.0 * right + bottomRight;
                vec4 sy = -topLeft - 2.0 * top - topRight + bottomLeft + 2.0 * bottom + bottomRight;
                outColor = sqrt(sx * sx + sy * sy);
            } 
            else {
                // Default: just pass through
                outColor = texture(texFramebuffer, Texcoord);
            }
        }
)glsl";
const GLchar* screenVertexSource = R"glsl(
        #version 330 core
        in vec2 position;
        in vec2 texcoord;
        out vec2 Texcoord;
        void main()
        {
            Texcoord = texcoord;
            gl_Position = vec4(position, 0.0, 1.0);
        }
    )glsl";
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
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    // Ask for OpenGL 3.3 Core Profile (common baseline)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a window
    GLFWwindow* window = glfwCreateWindow(800, 600, "GLEW + GLFW Test", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    // Make context current before using GLEW
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = GL_TRUE; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
        return -1;
    }

    //Depth tests are good for removing objects behind other objects, Stenci tests are good for outlining objects/shapes to make mirrors, windows, and masking models
    //Tests Depths to make sure not overlapping objects are drawn
    glEnable(GL_DEPTH_TEST);
    //Stencil buffer makes a map of zeros and draws objects changing the values of the map to one determined on the drawn model
    glEnable(GL_STENCIL_TEST);
    // Create some primitive



    //Unsigned int elements referring to vertices bound to GL_ARRAY_BUFFER if we want to draw them in order
    GLuint elements[] = {
        0, 1, 2,
        2, 3, 0
    };
    // Create VAOsGLint success;

    GLuint vaoQuad;
    glGenVertexArrays(1, &vaoQuad);
    // Create Vertix Buffer Objects (VBOs)
    GLuint vboQuad;
    glGenBuffers(1, &vboQuad);
    // Create shader programs
    GLuint sceneVertexShader, sceneFragmentShader, sceneShaderProgram, gridShaderProgram, gridVertexShader, gridFragmentShader;

    GLuint screenVertexShader, screenFragmentShader, screenShaderProgram;
    createShaderProgram(screenVertexSource, screenFragmentSource, screenVertexShader, screenFragmentShader, screenShaderProgram);
    createShaderProgram(sceneVertexSource, sceneFragmentSource, sceneVertexShader, sceneFragmentShader, sceneShaderProgram);

    // Create grid shaders
    gridVertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(gridVertexShader, 1, &sceneGridLinesVertexSource, NULL);
    glCompileShader(gridVertexShader);

    gridFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(gridFragmentShader, 1, &sceneGridLinesFragmentSource, NULL);
    glCompileShader(gridFragmentShader);

    createShaderProgram(gridVertexShader, gridFragmentShader, gridShaderProgram);

    glBindBuffer(GL_ARRAY_BUFFER, vboQuad);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);


    glBindVertexArray(vaoQuad);
    glBindBuffer(GL_ARRAY_BUFFER, vboQuad);
    specifyScreenVertexAttributes(screenShaderProgram);

    // Create grid VAO and VBO
    unsigned int gridVAO, gridVBO;
    glGenVertexArrays(1, &gridVAO);
    glGenBuffers(1, &gridVBO);

    glBindVertexArray(gridVAO);

    glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0); // unbind




    // Create framebuffer
    GLuint frameBuffer;
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

    // Create texture to hold color buffer
    GLuint texColorBuffer;
    glGenTextures(1, &texColorBuffer);
    glBindTexture(GL_TEXTURE_2D, texColorBuffer);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);

    // Create Renderbuffer Object to hold depth and stencil buffers
    GLuint rboDepthStencil;
    glGenRenderbuffers(1, &rboDepthStencil);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepthStencil);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboDepthStencil);




    glUseProgram(sceneShaderProgram);


    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 20000.0f);
    GLint uniProj = glGetUniformLocation(sceneShaderProgram, "proj");
    glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

    GLint uniColor = glGetUniformLocation(sceneShaderProgram, "overrideColor");

    glUseProgram(screenShaderProgram);
    GLint selection = glGetUniformLocation(screenShaderProgram, "selector");
    int curSelector = 0;
    glUniform1i(selection, curSelector);
    GLenum e = glGetError();
    if (e != GL_NO_ERROR) std::cerr << "GL error after uniform1i: " << e << std::endl;

    GLint uniView = glGetUniformLocation(sceneShaderProgram, "view");
    Camera cam = Camera(window);
    cam.SetCamera(window); // keep your initialization
    // store pointer for callback
    glfwSetWindowUserPointer(window, &cam);
    glfwSetCursorPosCallback(window, CameraCallback);
    while (!glfwWindowShouldClose(window)) {
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
        glEnable(GL_DEPTH_TEST);

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(sceneShaderProgram);
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) curSelector = 1;
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) curSelector = 2;
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) curSelector = 3;
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) curSelector = 4;
        if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) curSelector = 0;

        cam.CameraUpdate(window, deltaTime, sceneShaderProgram);


        
        glm::mat4 model = glm::mat4(1.0f);

        //Draw Grid
        glUseProgram(gridShaderProgram);

        glm::mat4 view = cam.GetViewMatrix(); // or however your Camera exposes it
        glUseProgram(gridShaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(gridShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(gridShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(proj));
        // set uniforms
        glUniformMatrix4fv(glGetUniformLocation(gridShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(gridShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(proj));


        glm::mat4 gridModel = glm::mat4(6.0f);
        glUniformMatrix4fv(glGetUniformLocation(gridShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(gridModel));


        // draw grid quad
        glDisable(GL_STENCIL_TEST);
        glBindVertexArray(gridVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);



        // Bind default framebuffer and draw contents of our framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindVertexArray(vaoQuad);
        glDisable(GL_DEPTH_TEST);
        glUseProgram(screenShaderProgram);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texColorBuffer);

        glUseProgram(screenShaderProgram);
        glUniform1i(selection, curSelector); GLint ok;
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glGetProgramiv(screenFragmentShader, GL_LINK_STATUS, &ok);
        if (!ok) {
            char buf[1024]; glGetProgramInfoLog(screenFragmentShader, 1024, NULL, buf);
            std::cerr << "LINK ERROR: " << buf << std::endl;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteRenderbuffers(1, &rboDepthStencil);
    glDeleteTextures(1, &texColorBuffer);
    glDeleteFramebuffers(1, &frameBuffer);

    glDeleteProgram(screenShaderProgram);
    glDeleteShader(screenFragmentShader);
    glDeleteShader(screenVertexShader);

    glDeleteProgram(sceneShaderProgram);
    glDeleteShader(sceneFragmentShader);
    glDeleteShader(sceneVertexShader);
    glDeleteShader(gridVertexShader);
    glDeleteShader(gridFragmentShader);

    glDeleteBuffers(1, &vboQuad);

    glDeleteVertexArrays(1, &vaoQuad);

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
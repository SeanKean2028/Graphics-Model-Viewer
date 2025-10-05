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

class Camera {

    glm::vec3 direction;
    bool firstMouse;
    float lastX;
    float lastY;
    float yaw;
    float pitch;
    float xPos;
    float yPos;
    glm::vec3 cameraTarget;
    glm::vec3 cameraDirection;
    glm::vec3 up;
    glm::vec3 cameraRight;
    glm::vec3 cameraPos;
    glm::vec3 cameraFront;
    glm::vec3 cameraUp;
    public: Camera(GLFWwindow* window) {
        direction = glm::vec3(0.0f, 0.0f, -1.0f);
        cameraRight = glm::normalize(glm::cross(direction, up));
        cameraPos = glm::vec3(1.0f, 1.0f, 3.0f);
        //Gram-Schmidt process to find the right and up vectors of the camera
        //By creating a 3x3 matrix with the forward, right, and up vectors as columns, we can dfine a 4th axes to add translation and scaling
        cameraDirection = glm::normalize(cameraPos - cameraTarget);
        up = glm::vec3(0.0f, 1.0f, 0.0f);
        cameraRight = glm::normalize(glm::cross(up, cameraDirection));
        cameraFront = glm::normalize(glm::vec3(0.0f, 0.0f, 0.0f) - cameraPos);
        cameraUp = glm::cross(cameraDirection, cameraRight);
        cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
        pitch = 0;
        yaw = 180;
        lastY = 0;
        lastX = 0;
        firstMouse = true;
        SetCamera(window);
    }
    public: void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
        if (firstMouse)
        {
            cameraFront = glm::normalize(glm::vec3(0.0f, 0.0f, 0.0f) - cameraPos);
            lastX = xpos;
            lastY = ypos;
            yaw = glm::degrees(atan2(cameraFront.z, cameraFront.x)) - 90.0f;
            pitch = glm::degrees(asin(cameraFront.y));
            direction = cameraFront;
            firstMouse = false;
            return;
        }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos;
        lastX = xpos;
        lastY = ypos;

        float sensitivity = 0.1f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        yaw += xoffset;
        pitch += yoffset;

        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
        ;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(direction);

    }
    public: void processInput(GLFWwindow* window, float deltaTime)
    {

        float cameraSpeed = 2.5f * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cameraPos += cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cameraPos -= cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
    public : void SetCamera(GLFWwindow* window) {
        //Gram-Schmidt process to find the right and up vectors of the camera
        //By creating a 3x3 matrix with the forward, right, and up vectors as columns, we can dfine a 4th axes to add translation and scaling
        cameraDirection = glm::normalize(cameraPos - cameraTarget);
        up = glm::vec3(0.0f, 1.0f, 0.0f);
    
        cameraRight = glm::normalize(glm::cross(cameraFront, up));
        cameraUp = glm::cross(cameraDirection, cameraRight);
        cameraFront = glm::normalize(glm::vec3(0.0f, 0.0f, 0.0f) - cameraPos);
        yaw = glm::degrees(atan2(cameraFront.z, cameraFront.x)) - 90.0f;
        pitch = glm::degrees(asin(cameraFront.y));


        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    void updateCameraVectors()
    {
        up = glm::vec3(0.0f, 1.0f, 0.0f);

        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(front);

        cameraRight = glm::normalize(glm::cross(cameraFront, up));
        cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));
    }
    public: void CameraUpdate(GLFWwindow* window, float deltaTime, GLuint shaderProgram) {
        processInput(window, deltaTime);
		updateCameraVectors();
        GLint uniView = glGetUniformLocation(shaderProgram, "view");

        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));
        float radius = 10.0f;
        float camX = sin(glfwGetTime() * radius);
        float camZ = cos(glfwGetTime() * radius);
    }
    glm::mat4 GetViewMatrix() const
    {
        return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    }
};
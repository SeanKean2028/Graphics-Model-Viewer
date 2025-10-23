#include<glm.hpp>
#include<vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "ShaderProgram.h"
using namespace std;
struct VertexAttribute {
    GLsizei stride;
    GLint amountOf;
    int offset;
    //Def Constructor: Stride between each vertex, amount of data in attribute, offset from last
    VertexAttribute(GLsizei _stride, GLint _amountOf, int _offset) {
        stride = _stride;
        amountOf = _amountOf;
        offset = _offset;
    }
};
class Mesh {    
public:

    float *vertices;
    int vertexCount;
    int floatsPerVertex;
    //Textures 
    float *textures; 
    
    //Vertex Array Object; Vertex Buffer Object
    GLuint VAO,VBO = 0;
    
    //Indices are used for reusing of vertices so we don't have to have 6 to make a rectangle we only need 4
    //The Indices show us which vertices are used for which corners 
    unsigned int* indices;
    vector<VertexAttribute> attributes;
    //EBO stores the Id of our Element buffer object
    GLuint EBO = 0;
    // Store vertex count (number of vertices, not floats)

    // Constructor:
    Mesh(float* _vertices, int _vertexCount, int _floatsPerVertex)
    {
        vertices = _vertices;
        vertexCount = _vertexCount;
        floatsPerVertex = _floatsPerVertex;
    }

    //Constructor used if using an element buffer
    Mesh(float *_vertices, unsigned int*_indices) {
        vertices = _vertices; indices = _indices;
    }
    void AddAttributePointer(VertexAttribute vertexAttribute) {
        attributes.push_back(vertexAttribute);
    }
    //Generates our mesh info and sets our 
    void GenerateMesh() {

        glBindVertexArray(0);

        glGenVertexArrays(1, &VAO); glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertexCount * floatsPerVertex * sizeof(float), vertices, GL_STATIC_DRAW);

        for (int i = 0; i < attributes.size(); i++) {
            glVertexAttribPointer(i, attributes[i].amountOf, GL_FLOAT, GL_FALSE, attributes[i].stride, (void*)(attributes[i].offset * sizeof(float)));
            glEnableVertexAttribArray(i);
        }

    }
    void Draw(ShaderProgram& shader, glm::mat4 viewMatrix) {
        // --- Draw Grid ---
        shader.use();
        glm::mat4 view = viewMatrix;
        shader.setMat4("view", view);
        shader.setMat4("model", glm::mat4(1.0f));

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        glBindVertexArray(0);   
    }
    void DrawEBO(ShaderProgram& shader, int ppSelector, GLuint texColorBuffer) {
        // ---------- Post-processing / Screen Quad ----------      
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);

        shader.use();
        shader.setInt("selector", ppSelector);

        glBindVertexArray(VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texColorBuffer);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
    void Deletion() {
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }
};  
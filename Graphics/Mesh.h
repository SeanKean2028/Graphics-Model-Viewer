#pragma once
#define GLEW_STATIC

#include<glm.hpp>
#include<vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "ShaderProgram.h"

using namespace std;

// Vertex structure
struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;

    Vertex() : Position(0), Normal(0), TexCoords(0) {};
    Vertex(glm::vec3 pos, glm::vec3 normal, glm::vec2 texCoords) {
        this->Position = pos;
        this->Normal = normal;
        this->TexCoords = texCoords;
    }
};

// Texture structure
struct Texture {
    unsigned int id;
    string type;
    string path;
};

struct VertexAttribute {
    GLsizei stride;
    GLint amountOf;
    int offset;
    VertexAttribute(GLsizei _stride, GLint _amountOf, int _offset) {
        stride = _stride;
        amountOf = _amountOf;
        offset = _offset;
    }
};

class Mesh {
public:
    float* vertices;
    int vertexCount;
    int floatsPerVertex;
    float* textures;
    int indexCount = 0;

    GLuint VAO, VBO = 0;
    unsigned int* indices;
    vector<VertexAttribute> attributes;
    vector<Texture> meshTextures;
    GLuint EBO = 0;

    // Constructor for Model loading (struct-based)
    Mesh(vector<Vertex> Vertices, vector<unsigned int> Indices, vector<Texture> Textures) {
        vertexCount = Vertices.size();
        floatsPerVertex = 8;

        // Convert vector to array
        vector<float> _vertices;
        meshTextures = Textures;
        for (Vertex vertex : Vertices) {
            _vertices.push_back(vertex.Position.x);
            _vertices.push_back(vertex.Position.y);
            _vertices.push_back(vertex.Position.z);
            _vertices.push_back(vertex.Normal.x);
            _vertices.push_back(vertex.Normal.y);
            _vertices.push_back(vertex.Normal.z);
            _vertices.push_back(vertex.TexCoords.x);
            _vertices.push_back(vertex.TexCoords.y);
        }

        // Allocate memory and copy data
        vertices = new float[_vertices.size()];
        for (size_t i = 0; i < _vertices.size(); i++) {
            vertices[i] = _vertices[i];
        }

        // Copy indices
        indices = new unsigned int[Indices.size()];
        for (size_t i = 0; i < Indices.size(); i++) {
            indices[i] = Indices[i];
        }
        indexCount = Indices.size();

        // Setup OpenGL buffers
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertexCount * floatsPerVertex * sizeof(float),
            vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned int),
            indices, GL_STATIC_DRAW);

        // Position attribute (location = 0)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Normal attribute (location = 1)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // TexCoord attribute (location = 2)
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
    }

    // Constructor for simple meshes (array-based)
    Mesh(float* _vertices, int _vertexCount, int _floatsPerVertex) {
        vertices = _vertices;
        vertexCount = _vertexCount;
        floatsPerVertex = _floatsPerVertex;
    }

    // Constructor for indexed meshes (array-based)
    Mesh(float* _vertices, int _vertexCount, int _floatsPerVertex, unsigned int* _indices, int _indiceSize) {
        vertices = _vertices;
        indices = _indices;
        vertexCount = _vertexCount;
        floatsPerVertex = _floatsPerVertex;
        indexCount = _indiceSize;
    }

    void AddAttributePointer(VertexAttribute vertexAttribute) {
        attributes.push_back(vertexAttribute);
    }

    void GenerateMesh() {
        glBindVertexArray(0);
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertexCount * floatsPerVertex * sizeof(float), vertices, GL_STATIC_DRAW);

        for (size_t i = 0; i < attributes.size(); i++) {
            glVertexAttribPointer(i, attributes[i].amountOf, GL_FLOAT, GL_FALSE,
                attributes[i].stride, (void*)(attributes[i].offset * sizeof(float)));
            glEnableVertexAttribArray(i);
        }
    }

    void GenerateEbos(ShaderProgram& shader) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertexCount * floatsPerVertex * sizeof(float),
            vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned int),
            indices, GL_STATIC_DRAW);

        for (size_t i = 0; i < attributes.size(); i++) {
            glVertexAttribPointer(i, attributes[i].amountOf, GL_FLOAT, GL_FALSE,
                attributes[i].stride, (void*)(attributes[i].offset * sizeof(float)));
            glEnableVertexAttribArray(i);
        }

        glBindVertexArray(0);
    }

    void GenerateEboQuads(ShaderProgram& shader) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertexCount * floatsPerVertex * sizeof(float),
            vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned int),
            indices, GL_STATIC_DRAW);

        shader.SetAttributePointers();

        glBindVertexArray(0);
    }

    // Draw method for models
    void DrawMesh(ShaderProgram& shader, Texture currTexture) {
        shader.use();
        //Set Up Textures
        glBindTexture(GL_TEXTURE_2D, currTexture.id);

        glBindVertexArray(VAO);
        if (indexCount > 0) {
            glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        }
        else {
            glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        }
        glBindVertexArray(0);
    }
    void DrawMesh(ShaderProgram& shader)
    {
        shader.use();

        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;

        for (unsigned int i = 0; i < meshTextures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);

            string number;
            string name = meshTextures[i].type;

            if (name == "texture_diffuse")
                number = std::to_string(diffuseNr++);
            else if (name == "texture_specular")
                number = std::to_string(specularNr++);

            shader.setInt((name + number).c_str(), i);

            glBindTexture(GL_TEXTURE_2D, meshTextures[i].id);
        }

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glActiveTexture(GL_TEXTURE0);
    }
    // Draw method for simple meshes
    void DrawSimple(ShaderProgram& shader) {
        shader.use();
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        glBindVertexArray(0);
    }

    void DrawPostProcessing(ShaderProgram& shader, int ppSelector, GLuint texColorBuffer) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);

        shader.use();
        shader.setInt("selector", ppSelector);
        shader.setInt("screenTexture", 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texColorBuffer);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void DrawEBO(ShaderProgram& shader, GLuint texColorBuffer) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);

        shader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texColorBuffer);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void Deletion() {
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }

    void EBODeletion() {
        if (EBO) glDeleteBuffers(1, &EBO);
        if (VBO) glDeleteBuffers(1, &VBO);
        if (VAO) glDeleteVertexArrays(1, &VAO);
    }

    ~Mesh() {
        // Note: Be careful with cleanup if using pointer assignment
        // Only delete if we allocated memory in constructor
    }
};
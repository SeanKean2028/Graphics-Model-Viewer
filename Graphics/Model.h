#pragma once
#include "ShaderProgram.h"
#include "Mesh.h"  // This includes Vertex and Texture structs
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stb_image.h>

#include <iostream>
#include <vector>
#include <string>
#include <cstring>

using namespace std;

class Model {
public:
    // Constructor - takes only path
    Model(const std::string& path) {
        loadModel(path);
    }

    Model(const char* path) {
        loadModel(std::string(path));
    }

    // Draw all meshes
    void Draw(ShaderProgram& shader) {
        if (meshes.empty()) {
            cout << "[Model] Warning: no meshes to draw\n";
            return;
        }
        for (unsigned int i = 0; i < meshes.size(); i++) {
            meshes[i].DrawMesh(shader);
        }
    }

    // Get mesh count for debugging
    size_t getMeshCount() const { return meshes.size(); }

private:
    // Model data
    vector<Mesh> meshes;
    vector<Texture> textures_loaded;
    string directory;

    // Load model using Assimp
    void loadModel(const string& path) {
        Assimp::Importer import;
        const aiScene* scene = import.ReadFile(path,
            aiProcess_Triangulate |
            aiProcess_FlipUVs |
            aiProcess_CalcTangentSpace);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            cout << "ERROR::ASSIMP::" << import.GetErrorString() << " -- path: " << path << endl;
            return;
        }

        cout << "Model loaded successfully: " << path << endl;

        // Handle both '/' and '\' directory separators (Windows)
        size_t pos = path.find_last_of("/\\");
        if (pos != string::npos)
            directory = path.substr(0, pos);
        else
            directory = ".";

        processNode(scene->mRootNode, scene);

        cout << "Total meshes loaded: " << meshes.size() << endl;
    }

    // Traverse scene nodes
    void processNode(aiNode* node, const aiScene* scene) {
        cout << "Processing node: " << node->mName.C_Str() << endl;

        // Process all the node's meshes
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }

        // Then process children
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            processNode(node->mChildren[i], scene);
        }
    }

    // Convert aiMesh to our Mesh
    Mesh processMesh(aiMesh* mesh, const aiScene* scene) {
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;

        cout << "  Mesh - Verts: " << mesh->mNumVertices
            << " Faces: " << mesh->mNumFaces
            << " UVs: " << (mesh->HasTextureCoords(0) ? "Yes" : "No")
            << " Normals: " << (mesh->HasNormals() ? "Yes" : "No")
            << endl;

        // Process vertices
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex;

            // Positions
            glm::vec3 vectorPos(0.0f);
            if (mesh->mVertices) {
                vectorPos.x = mesh->mVertices[i].x;
                vectorPos.y = mesh->mVertices[i].y;
                vectorPos.z = mesh->mVertices[i].z;
            }
            vertex.Position = vectorPos;

            // Normals (check existence)
            glm::vec3 normal(0.0f);
            if (mesh->HasNormals() && mesh->mNormals) {
                normal.x = mesh->mNormals[i].x;
                normal.y = mesh->mNormals[i].y;
                normal.z = mesh->mNormals[i].z;
            }
            vertex.Normal = normal;

            // Texture coordinates (only first set used)
            if (mesh->HasTextureCoords(0)) {
                glm::vec2 tex;
                tex.x = mesh->mTextureCoords[0][i].x;
                tex.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = tex;
            }
            else {
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            }

            vertices.push_back(vertex);
        }

        // Process indices (faces)
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }

        // Process material
        if (mesh->mMaterialIndex < scene->mNumMaterials) {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

            // Diffuse maps
            vector<Texture> diffuseMaps = loadMaterialTextures(material,
                aiTextureType_DIFFUSE, "texture_diffuse");
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

            // Specular maps
            vector<Texture> specularMaps = loadMaterialTextures(material,
                aiTextureType_SPECULAR, "texture_specular");
            textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        }

        // Return Mesh with only 3 arguments
        return Mesh(vertices, indices, textures);
    }

    // Load textures for a material with path sanitization
    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName) {
        vector<Texture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
            aiString str;
            mat->GetTexture(type, i, &str);

            // Raw path from Assimp
            std::string texPath = str.C_Str();

            // Normalize slashes
            std::replace(texPath.begin(), texPath.end(), '\\', '/');

            // Strip absolute path → keep only filename
            size_t pos = texPath.find_last_of('/');
            if (pos != std::string::npos) {
                texPath = texPath.substr(pos + 1); // e.g. "Characters.png"
            }

            // Build final path relative to your model directory
            std::string fullPath = directory + '/' + texPath;

            // Check if already loaded
            bool skip = false;
            for (const auto& loaded : textures_loaded) {
                if (loaded.path == texPath) {
                    textures.push_back(loaded);
                    skip = true;
                    break;
                }
            }

            if (!skip) {
                Texture texture;
                texture.id = TextureFromFile(fullPath.c_str(), directory, false);
                texture.type = typeName;
                texture.path = texPath; // store just the filename for comparison
                textures.push_back(texture);
                textures_loaded.push_back(texture);

                std::cout << "[loadMaterialTextures] Loaded: " << fullPath << std::endl;
            }
        }
        return textures;
    }
    unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma) {
        std::string filename(path);

        // Normalize Windows backslashes to forward slashes
        std::replace(filename.begin(), filename.end(), '\\', '/');

        // Check if path is absolute (Windows drive letter or Unix-style root)
        bool isAbsolute = (filename.find(':') != std::string::npos) ||
            (!filename.empty() && filename[0] == '/');


        std::cout << "[TextureFromFile] Loading: " << filename << std::endl;

        unsigned int textureID;
        glGenTextures(1, &textureID);

        int width, height, nrComponents;
        unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);

        if (data) {
            GLenum format = GL_RGB;
            if (nrComponents == 1) format = GL_RED;
            else if (nrComponents == 3) format = GL_RGB;
            else if (nrComponents == 4) format = GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);

            std::cout << "[TextureFromFile] OK: " << width << "x" << height
                << " channels=" << nrComponents << std::endl;
        }
        else {
            std::cerr << "[TextureFromFile] Failed to load: " << filename << std::endl;
        }

        return textureID;
    }

};
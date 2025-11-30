#pragma once
#include <vector>
#include <cmath>
#include "Mesh.h"
using namespace std;

class Sphere {
    // clear memory of prev arrays
public: vector<float> vertices;
    vector<float> normals;
    vector<float> texCoords;
    vector<Vertex> verticesMesh;
    vector<unsigned int> indices;
    vector<int> lineIndices;
private:
    //Conversion to Vertex in mesh
    vector<glm::vec3> positions;
    vector<glm::vec3> norms;
    vector<glm::vec2> texCorodinates;
    float radius;
    
    const double PI = 2 * acos(0.0);
    float sectorCount;
    float stackCount;

    float x, y, z, xy;                              // vertex position
    float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
    float s, t;                                     // vertex texCoord

public: Sphere(float _radius, float _stackCount, float _sectorCount) {
        radius = _radius;
        stackCount = _stackCount;
        sectorCount = _sectorCount;
        float sectorStep = 2 * PI / sectorCount;
        float stackStep = PI / stackCount;
        float sectorAngle, stackAngle;


        for (int i = 0; i <= stackCount; ++i)
        {
            stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
            xy = radius * cosf(stackAngle);             // r * cos(u)
            z = radius * sinf(stackAngle);              // r * sin(u)

            // add (sectorCount+1) vertices per stack
            // first and last vertices have same position and normal, but different tex coords
            for (int j = 0; j <= sectorCount; ++j)
            {
                sectorAngle = j * sectorStep;           // starting from 0 to 2pi

                // vertex position (x, y, z)
                x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
                y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);
                positions.push_back(glm::vec3(x,y,z));
                // normalized vertex normal (nx, ny, nz)
                nx = x * lengthInv;
                ny = y * lengthInv;
                nz = z * lengthInv;
                normals.push_back(nx);
                normals.push_back(ny);
                normals.push_back(nz);
                norms.push_back(glm::vec3(nx, ny, nz));
                // vertex tex coord (s, t) range between [0, 1]
                s = (float)j / sectorCount;
                t = (float)i / stackCount;
                texCoords.push_back(s);
                texCoords.push_back(t);
                texCorodinates.push_back(glm::vec2(s, t));
            }

        }
        cout << "Sphere Vertices Instantiated " << endl;
        // generate CCW index list of sphere triangles
// k1--k1+1
// |  / |
// | /  |
// k2--k2+1
        for (int i = 0; i < positions.size(); i++) {
            verticesMesh.push_back(Vertex(positions[i], norms[i], texCorodinates[i]));
        }
        cout << "Sphere Mesh.h Vertices Instantiated " << endl;
        int k1, k2;
        for (int i = 0; i < stackCount; ++i)
        {
            k1 = i * (sectorCount + 1);     // beginning of current stack
            k2 = k1 + sectorCount + 1;      // beginning of next stack

            for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
            {
                // 2 triangles per sector excluding first and last stacks
                // k1 => k2 => k1+1
                if (i != 0)
                {
                    indices.push_back(k1);
                    indices.push_back(k2);
                    indices.push_back(k1 + 1);
                }

                // k1+1 => k2 => k2+1
                if (i != (stackCount - 1))
                {
                    indices.push_back(k1 + 1);
                    indices.push_back(k2);
                    indices.push_back(k2 + 1);
                }

                // store indices for lines
                // vertical lines for all stacks, k1 => k2
                lineIndices.push_back(k1);
                lineIndices.push_back(k2);
                if (i != 0)  // horizontal lines except 1st stack, k1 => k+1
                {
                    lineIndices.push_back(k1);
                    lineIndices.push_back(k1 + 1);
                }
            }
        }
        cout << "Sphere instantiated! " << endl;
    }
};
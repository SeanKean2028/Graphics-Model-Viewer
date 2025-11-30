#version 330 core

layout (location = 0) in vec3 aPos;  // position from your ground mesh

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 worldPos;

void main()
{
    vec4 world = model * vec4(aPos, 1.0);
    worldPos = world.xyz;  // pass to fragment shader
    gl_Position = projection * view * world;
}
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
#version 410 core

// uniform inputs
uniform mat4 projection;
uniform mat4 view;

// attribute inputs
layout(location = 0) in vec3 aPos;

// varying outputs
out vec3 TexCoords;

void main() {
    TexCoords = aPos;
    vec4 pos = projection * view * vec4(aPos, 1.0);
    gl_Position = pos.xyww;     // causes norm dev coords to always have a z value equal to 1.0, or max depth value
}
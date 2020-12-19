#version 410 core

// uniform inputs
uniform samplerCube skybox;

// varying inputs
in vec3 TexCoords;

// outputs
out vec4 fragColorOut;

void main() {
    fragColorOut = texture(skybox, TexCoords);
}
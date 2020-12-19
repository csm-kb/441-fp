#version 410 core

layout (triangles) in;
layout (triangle_strip, max_vertices=3) out;

uniform mat4 mvpMatrix;

layout(location = 0) in vec3 vPos[3];
layout(location = 1) in vec3 vNorm[3];
layout(location = 2) in float jitterStrength[];

layout(location = 0) out vec3 vposOut;
layout(location = 1) out vec3 normOut;

float random(vec2 co) {
    return fract(sin(dot(co.xy,vec2(12.9898,78.233))) * 43758.5453);
}

void main() {
    for (int i = 0; i < 3; i++) {
        gl_Position = gl_in[i].gl_Position + vec4(vec3(random(gl_in[i].gl_Position.xy))*vec3(jitterStrength[0]),1.0);
        vposOut = vPos[i];
        normOut = vNorm[i];
        EmitVertex();
    }

    EndPrimitive();
}

/*
 *   Geometry Shader
 *
 *   CSCI 441, Computer Graphics, Colorado School of Mines
 */

#version 410 core

// TODO #A
layout ( points ) in;

// TODO #B
layout ( triangle_strip, max_vertices = 4 ) out;

vec4 vecs[4] = vec4[](
    vec4(-1,-1, 0,1),
    vec4(-1, 1, 0,1),
    vec4( 1,-1, 0,1),
    vec4( 1, 1, 0,1)
);

uniform mat4 projMatrix;

vec2 texCoords[4] = vec2[](
    vec2(0,0),
    vec2(1,0),
    vec2(0,1),
    vec2(1,1)
);

// TODO #I
out vec2 texCoord;

void main() {
    // TODO #C
    for (int i = 0; i < 4; i++) {
        gl_Position = projMatrix * (gl_in[0].gl_Position + vecs[i]);
        texCoord = texCoords[i];
        EmitVertex();
    }
    EndPrimitive();
}

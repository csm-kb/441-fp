#version 410 core

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNorm;

uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;
uniform mat4 viewMtx;
uniform mat4 projMtx;
uniform mat4 modelMtx;
uniform mat4 normalMtx;
uniform float jitterStrength;

layout(location = 0) out vec3 vertNorm;
layout(location = 1) out vec3 vertPos;
layout(location = 2) out float _jitterStr;

void main() {
    gl_Position = mvpMatrix * vec4(vPos,1.0);
    _jitterStr = jitterStrength;
    vec3 cameraVec = normalize( -(modelMtx * vec4(vPos,1.0)).xyz );
    vec4 vertPos4 = modelMtx * vec4(vPos,1.0);
    vertPos = vec3(vertPos4) / vertPos4.w;
    vertNorm = normalize( (normalMtx * vec4(vNorm,0.0) ).xyz);
}

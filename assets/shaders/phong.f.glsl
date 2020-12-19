#version 410 core

uniform vec3 materialDiffColor;
uniform vec3 materialSpecColor;
uniform float materialShininess;
uniform vec3 materialAmbColor;
//uniform sampler2D txtr;

//layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec3 normalVec;
layout(location = 4) in vec3 lightVec;
layout(location = 5) in vec3 halfwayVec;

layout(location = 0) out vec4 fragColorOut;

const vec3 LIGHT_DIFFUSE = vec3(1.0, 1.0, 1.0);
const vec3 LIGHT_SPECULAR = vec3(1.0, 1.0, 1.0);
const vec3 LIGHT_AMBIENT = vec3(1.0, 1.0, 1.0);

void main() {
    vec3 lightVec2 = normalize(lightVec);
    vec3 normalVec2 = normalize(normalVec);
    vec3 halfwayVec2 = normalize(halfwayVec);
    
    float sDotN = max( dot(lightVec2, normalVec2), 0.0 );
    vec3 diffuse = LIGHT_DIFFUSE * materialDiffColor * sDotN;
    
    vec3 specular = vec3(0.0);
    if( sDotN > 0.0 )
        specular = LIGHT_SPECULAR * materialSpecColor * pow( max( 0.0, dot( halfwayVec2, normalVec2 ) ), materialShininess );
    
    vec3 ambient = LIGHT_AMBIENT * materialAmbColor;
    
    fragColorOut = vec4(diffuse + specular + ambient, 1.0);
    
//    vec4 texel = texture( txtr, texCoord );
//    fragColorOut *= texel;
}

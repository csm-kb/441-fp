#version 410 core

layout(location = 0) in vec3 vertNorm;
layout(location = 1) in vec3 vertPos;
layout(location = 0) out vec4 color;

uniform mat4 viewMtx;                   // view matrix
uniform vec3 eyePos;                    // eye position in world space
uniform vec3 materialDiffColor;         // the material diffuse color
uniform vec3 materialSpecColor;         // the material specular color
uniform float materialShininess;        // the material shininess value
uniform vec3 materialAmbColor;          // the material ambient color

const float screenGamma = 2.2; // sRGB gamma correction

const float lightPower = 40.0; // debug

struct Light {
    int lightType;      // 0 - point light, 1 - directional light, 2 - spotlight
    vec3 lightPos;      // light position in world space
    vec3 lightDir;      // light direction in world space
    float lightCutoff;  // angle of our spotlight
    vec3 lightColor;    // light color
};
#define N_LIGHTS 1
uniform Light lights[N_LIGHTS];

//float min3(vec3 v) { return min(min(v.x,v.y),v.z); }
//float max3(vec3 v) { return max(max(v.x,v.y),v.z); }
float bug = 0.0;

void main() {
    vec3 colorLinear = materialAmbColor;
    for (int i = 0; i < N_LIGHTS; i++) {
        vec3 viewDir = normalize((/*viewMtx */ vec4(eyePos, 1.0)).xyz - vertPos);
        vec3 lightDir = vec3(0.0);
        if (lights[i].lightType == 1) // directional lights
            lightDir = normalize( (/*viewMtx */ vec4(lights[i].lightDir, 1.0)).xyz );
        else // spot/point lights
            lightDir = normalize( (/*viewMtx */ vec4(lights[i].lightPos, 1.0)).xyz - vertPos );
        float dist = length(lightDir);
        lightDir = lightDir / dist;
        dist = dist * dist;

        float NdotL = dot(vertNorm, lightDir);
        float intensity = clamp(NdotL, 0.0, 1.0);
        colorLinear += intensity * materialDiffColor * lights[i].lightColor * lightPower / dist;

        vec3 H = normalize(viewDir + lightDir);
        float NdotH = dot(vertNorm, H);
        intensity = pow(clamp(NdotH, 0.0, 1.0), materialShininess);
        colorLinear += intensity * materialSpecColor * lights[i].lightColor * lightPower / dist;
    }
    colorLinear = clamp(colorLinear, 0.0, 1.0);
    // apply gamma correction
    vec3 colorGammaCorr = pow(colorLinear, vec3(1.0/screenGamma));
    color = vec4(colorGammaCorr,1.0);
    // for debugging
    color.x += bug;
}

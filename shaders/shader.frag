#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColour;
layout(location = 1) in vec3 normalForFP;
layout(location = 2) in vec3 positionForFP;
layout(location = 3) in vec3 lightPos;
layout(location = 4) in vec2 fragtex;
layout(location = 5) in mat4 fragM;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 lightDirection = normalize(lightPos - positionForFP );
    vec3 viewDirection = normalize(-positionForFP );
    vec3 halfVector = normalize( lightDirection + viewDirection);

    float diffuse = max(0.0f,dot( normalForFP.xyz, lightDirection));
    float specular = max(0.0f,dot( normalForFP.xyz, halfVector ) );
    float distanceFromLight = length(lightPos - positionForFP);

    if (diffuse == 0.0) {
        specular = 0.0;
    } else {
        specular = pow( specular, 64.0f );
    }

    vec3 scatteredLight = 1.0f/distanceFromLight * fragColour * diffuse;
    vec3 reflectedLight = vec3(1.0f,1.0f,1.0f) * specular;
    vec3 ambientLight = fragColour * 0.01f;

    outColor = vec4(min( ambientLight + scatteredLight + reflectedLight, vec3(1,1,1)), 1.0);
}

﻿#version 440
in vec2 texCoord;
out vec4 FragColor;

uniform sampler2D gBufferPosition;
uniform sampler2D gBufferNormal;
uniform sampler2D gBufferAlbedo;
uniform sampler2D gBufferEmissive;
uniform sampler3D voxelTex0; // Pos.xyz + Metallic
uniform sampler3D voxelTex1; // Normal.xyz + Smoothness
uniform sampler3D voxelTex2; // Albedo.rgb + EmissiveFactor
uniform int uDebugIndex;
uniform int uVoxelRes;
uniform float uVoxelWorldSize;

const float CONE_APERTURE = 0.6;
const float STEP_MULTIPLIER = 3;
const float MAX_STEPS = 64;
const float EMISSIVE_THRESHOLD = 0.001;
float VOXEL_SIZE = uVoxelWorldSize / float(uVoxelRes);
const int NUM_CONES = 16;
const float BRIGHTNESS_MULTIPLIER = 200.0;
 

vec3 worldToVoxel(vec3 pos) {
    return (pos / uVoxelWorldSize) + 0.5;
}

float calculateOcclusion(vec3 normal) { // we can estimate occlusion based off of the normal length, where 1 = full occlusion, 0.2 partial occlusion and ~0 = no occlusion
    return length(normal);
}

int debugPass(vec3 worldPos, float metallic, vec3 worldNormal, float smoothness,
    vec3 albedo, float emissiveFactor, vec3 emissiveRgb, float spare) {
    if (uDebugIndex == 1)      FragColor = vec4(worldPos, 1.0);
    else if (uDebugIndex == 2) FragColor = vec4(metallic);
    else if (uDebugIndex == 3) FragColor = vec4(worldNormal, 1.0);
    else if (uDebugIndex == 4) FragColor = vec4(smoothness);
    else if (uDebugIndex == 5) FragColor = vec4(albedo, 1.0);
    else if (uDebugIndex == 6) FragColor = vec4(emissiveFactor);
    else if (uDebugIndex == 7) FragColor = vec4(emissiveRgb, 1.0);
    else if (uDebugIndex == 8) FragColor = vec4(spare);
    else if (uDebugIndex == 9) FragColor = vec4(texture(voxelTex2, worldToVoxel(worldPos)).xyz, 1);
    else if (uDebugIndex == 10) FragColor = vec4(vec3(calculateOcclusion(texture(voxelTex1, worldToVoxel(worldPos)).xyz)), 1);
    else return 0;
    return 1;
}

vec3 traceCone(vec3 origin, vec3 direction, float aperature) {
    vec3 accumulatedLight = vec3(0.0);
    float distance = VOXEL_SIZE * 3.0;
    for (int i = 0; i < MAX_STEPS; i++) {
        vec3 samplePos = origin + direction * distance;
        vec3 sampleCoord = worldToVoxel(samplePos);
        if (any(lessThan(sampleCoord, vec3(0.0))) || any(greaterThan(sampleCoord, vec3(1.0))))
            break;
        float coneDiameter = distance * aperature;
        float mipLevel = clamp(log2(coneDiameter / VOXEL_SIZE), 0.0, 6.0);
        // sample voxel data
        vec4 voxelData = textureLod(voxelTex2, sampleCoord, mipLevel);
        float voxelEmissive = voxelData.a;

        // Accumulate emissive contribution
        if (voxelEmissive > EMISSIVE_THRESHOLD)
            accumulatedLight += voxelData.rgb * voxelEmissive;
        distance += VOXEL_SIZE * coneDiameter * STEP_MULTIPLIER;
    }
    return accumulatedLight;
}
void getTangentSpace(vec3 normal, out vec3 tangent, out vec3 bitangent) {
    vec3 up = abs(normal.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    tangent = normalize(cross(up, normal));
    bitangent = cross(normal, tangent);
}
vec3 sampleHemisphere(vec3 origin, vec3 normal) {
    vec3 tangent, bitangent;
    getTangentSpace(normal, tangent, bitangent);
    vec3 totalLight = vec3(0.0);
    for (int i = 0; i < NUM_CONES; i++) {
        float angle = float(i) * 2.0 * 3.14159265 / float(NUM_CONES);
        float elevation = 0.5;
        float radius = sqrt(1.0 - elevation * elevation);
        vec3 direction = normalize(
            tangent * (cos(angle) * radius) +
            bitangent * (sin(angle) * radius) +
            normal * elevation
        );
        float weight = max(dot(direction, normal), 0.0);
        vec3 coneLight = traceCone(origin, direction, CONE_APERTURE);
        totalLight += coneLight * weight;
    }
    return totalLight / float(NUM_CONES) * BRIGHTNESS_MULTIPLIER;
}

void main() {
    vec3 worldPos = texture(gBufferPosition, texCoord).xyz;
    float metallic = texture(gBufferPosition, texCoord).w;
    vec3 worldNormal = texture(gBufferNormal, texCoord).xyz;
    float smoothness = texture(gBufferNormal, texCoord).w;
    vec3 albedo = texture(gBufferAlbedo, texCoord).xyz;
    float emissiveFactor = texture(gBufferAlbedo, texCoord).w;
    vec3 emissiveRgb = texture(gBufferEmissive, texCoord).xyz;
    float spare = texture(gBufferEmissive, texCoord).w;

    if (debugPass(worldPos, metallic, worldNormal, smoothness, albedo, emissiveFactor, emissiveRgb, spare) == 1)
        return;

    if (length(worldNormal) < 0.1) // fragment is the sky
        FragColor = vec4(albedo, 1.0);

    // trace cone to gather direct emissive light
    vec3 directLight = sampleHemisphere(worldPos, worldNormal);
    vec3 specular = traceCone(worldPos, worldNormal, CONE_APERTURE) * BRIGHTNESS_MULTIPLIER / 4;

    // apply lighting to surface and add self-emissive
    vec3 outColor = albedo * (specular + directLight) + emissiveRgb * emissiveFactor + albedo * 0.1;

    FragColor = vec4(outColor, 1.0);
}
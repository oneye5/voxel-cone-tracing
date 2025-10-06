#version 440
in vec2 texCoord;
out vec4 FragColor;

uniform sampler2D gBufferPosition;
uniform sampler2D gBufferNormal;
uniform sampler2D gBufferAlbedo;
uniform sampler2D gBufferEmissive;
uniform sampler3D voxelTex0; // Pos.xyz + Metallic
uniform sampler3D voxelTex1; // Normal.xyz + Smoothness
uniform sampler3D voxelTex2; // Albedo.rgb + EmissiveFactor
uniform vec3 cameraPos;
uniform mat4 uViewMatrix;
uniform int uVoxelRes;
uniform float uVoxelWorldSize;
uniform vec3 uVoxelCenter;
uniform float VOXEL_SIZE;
uniform float uMipLevelCount;
uniform float uConeAperture; 
uniform float uStepMultiplier;
uniform float uMaxSteps;
uniform float uEmissiveThreshold;
uniform int   uNumDiffuseCones; 
uniform float uDiffuseBrightnessMultiplier;
uniform float uTransmittanceNeededForConeTermination;
uniform vec3  uAmbientColor;
uniform int uDebugIndex;
uniform float uReflectionBlendLowerBound;
uniform float uReflectionBlendUpperBound;

const float PI = 3.14159265359;
#define APERTURE_SCALE 1.0

/*
    FEATURES:
    Emissive based specular for rough materials, geometry based reflections for smooth, with smooth blending between the two

    Monte carlo diffuse GI, using cosine weighted hemisphere sampling

    Ambient occlusion from average transmittance

    Fresnel

    Metallics
*/  


vec3 worldToVoxel(vec3 pos) {
    return (pos - uVoxelCenter + uVoxelWorldSize * 0.5) / uVoxelWorldSize;
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
    else if (uDebugIndex == 10) FragColor = vec4(1);
    else return 0;
    return 1;
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void getTangentSpace(vec3 normal, out vec3 tangent, out vec3 bitangent) {
    vec3 up = abs(normal.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    tangent = normalize(cross(up, normal));
    bitangent = cross(normal, tangent);
}

float radicalInverse_VdC(uint bits) {       // 'converts' int into a float by reversing its binary representation
    bits = (bits << 16u) | (bits >> 16u);   // like a random number generator, but evenly distributed
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 2^32
}

// cos weighted sample on a hemisphere
vec3 cosineSampleHemisphere(float u1, float u2) {
    float r = sqrt(u1);
    float phi = 2.0 * PI * u2;
    return vec3(r * cos(phi), r * sin(phi), sqrt(max(0.0, 1.0 - u1)));
}

// standard trace cone function, traces against emissives
vec4 traceCone(vec3 origin, vec3 direction, float aperture, bool stopAtFirstHit) {
    vec3 accumulatedColor = vec3(0.0);
    float accumulatedAlpha = 0.0;
    float distance = VOXEL_SIZE * 2.0;

    for (int i = 0; i < int(uMaxSteps); ++i) {
        vec3 samplePos = origin + direction * distance;
        vec3 sampleCoord = worldToVoxel(samplePos);

        if (any(lessThan(sampleCoord, vec3(0.0))) || any(greaterThan(sampleCoord, vec3(1.0))))
            break;

        float coneDiameter = max(VOXEL_SIZE, distance * aperture);
        float mipLevel = clamp(log2(coneDiameter / VOXEL_SIZE), 0.0, uMipLevelCount);

        vec4 voxelData1 = textureLod(voxelTex1, sampleCoord, mipLevel);
        vec4 voxelData2 = textureLod(voxelTex2, sampleCoord, mipLevel);
        float occlusion = length(voxelData1.xyz);

        if (occlusion > 0.01) {
            vec3 voxelColor = voxelData2.rgb;
            float emissiveFactor = voxelData2.a;
            vec3 emissiveLight = voxelColor * emissiveFactor;

            float transmittance = 1.0 - accumulatedAlpha;
            accumulatedColor += emissiveLight * occlusion * transmittance;
            accumulatedAlpha += occlusion * transmittance;

            // For sharp reflections, we stop at the very first surface we find.
            if (stopAtFirstHit) {
                accumulatedColor = emissiveLight; // Return just the color of the first hit
                accumulatedAlpha = 1.0; // Mark as fully occluded
                break;
            }
        }

        if (accumulatedAlpha > (1.0 - uTransmittanceNeededForConeTermination))
            break;

        distance += coneDiameter * uStepMultiplier;
    }

    return vec4(accumulatedColor, 1.0 - accumulatedAlpha);
}

// Version of above that traces against all geometry instead of just emissives. This is used for reflections of the geometry. 
vec4 traceConeAgainstGeometry(vec3 origin, vec3 direction, float aperture) {
    vec3 accumulatedColor = vec3(0.0);
    float accumulatedAlpha = 0.0;
    float distance = VOXEL_SIZE * 2.0;

    for (int i = 0; i < int(uMaxSteps); ++i) {
        vec3 samplePos = origin + direction * distance;
        vec3 sampleCoord = worldToVoxel(samplePos);

        if (any(lessThan(sampleCoord, vec3(0.0))) || any(greaterThan(sampleCoord, vec3(1.0))))
            break;

        float coneDiameter = max(VOXEL_SIZE, distance * aperture);
        float mipLevel = clamp(log2(coneDiameter / VOXEL_SIZE), 0.0, uMipLevelCount);

        vec4 voxelData1 = textureLod(voxelTex1, sampleCoord, mipLevel);
        vec4 voxelData2 = textureLod(voxelTex2, sampleCoord, mipLevel);
        float occlusion = length(voxelData1.xyz);

        if (occlusion > 0.01) {
            vec3 voxelAlbedo = voxelData2.rgb;
            float emissiveFactor = voxelData2.a;
            vec3 voxelRadiance = voxelAlbedo + (voxelAlbedo * emissiveFactor);

            float transmittance = 1.0 - accumulatedAlpha;
            accumulatedColor += voxelRadiance * occlusion * transmittance;
            accumulatedAlpha += occlusion * transmittance;

            accumulatedColor = voxelRadiance;
            accumulatedAlpha = 1.0;
            break;
        }

        if (accumulatedAlpha > (1.0 - uTransmittanceNeededForConeTermination))
            break;

        distance += coneDiameter * uStepMultiplier;
    }

    return vec4(accumulatedColor, 1.0 - accumulatedAlpha);
}


// Monte carlo approach
vec4 indirectDiffuseLight(vec3 pos, vec3 normal) {
    vec3 tangent, bitangent;
    getTangentSpace(normal, tangent, bitangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    vec4 accumulatedResult = vec4(0.0);
    int numSamples = max(1, uNumDiffuseCones);

    uint baseSeed = floatBitsToUint(fract(dot(pos, vec3(12.9898, 78.233, 45.164))));

    for (int i = 0; i < numSamples; ++i) {
        float u1 = (float(i) + 0.5) / float(numSamples);
        float u2 = radicalInverse_VdC(baseSeed + uint(i)); 

        vec3 localDir = cosineSampleHemisphere(u1, u2);
        vec3 worldDir = TBN * localDir;

        // trace cone in direction
        accumulatedResult += traceCone(pos, worldDir, uConeAperture, false);
    }

    // return the average color and average transmittance from all samples
    return accumulatedResult / float(numSamples);
}

void main() {
    // read g buffer
    vec3 worldPos = texture(gBufferPosition, texCoord).xyz;
    float metallic = texture(gBufferPosition, texCoord).w;
    vec3 worldNormal = texture(gBufferNormal, texCoord).xyz;
    float smoothness = texture(gBufferNormal, texCoord).w;
    vec3 albedo = texture(gBufferAlbedo, texCoord).xyz;
    float emissiveFactor = texture(gBufferAlbedo, texCoord).w;
    vec3 emissiveRgb = texture(gBufferEmissive, texCoord).xyz;
    float spare = texture(gBufferEmissive, texCoord).w;

    // debug and early exit cases
    if (debugPass(worldPos, metallic, worldNormal, smoothness, albedo, emissiveFactor, emissiveRgb, spare) == 1) return;
    if (length(worldNormal) < 0.1) { FragColor = vec4(albedo, 1.0); return; }
    if (emissiveFactor > uEmissiveThreshold) { FragColor = vec4(emissiveRgb * emissiveFactor, 1.0); return; }

    // setup vars
    worldNormal = normalize(worldNormal);
    vec3 viewDir = normalize(cameraPos - worldPos);
    float roughness = 1.0 - smoothness;
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    float NdotV = max(dot(worldNormal, viewDir), 0.0);
    vec3 F = fresnelSchlickRoughness(NdotV, F0, roughness);
    vec3 kD = (1.0 - F) * (1.0 - metallic);
    vec3 traceOrigin = worldPos + worldNormal * VOXEL_SIZE * 2.0;

    // calculate indirect lighting
    vec4 indirectDiffuseResult = indirectDiffuseLight(traceOrigin, worldNormal);
    vec3 indirectDiffuse = indirectDiffuseResult.rgb;
    float ambientOcclusion = indirectDiffuseResult.a;     // the average transmittance from the diffuse cones gives a plausable ambient occlusion term

    // calculate specular and reflections
    vec3 reflectDir = reflect(-viewDir, worldNormal);
    // aperture based on roughness squared (physically accurate)
    float specularAperture = roughness * roughness * APERTURE_SCALE;
    specularAperture = clamp(specularAperture, 0.001, 0.5);
    vec3 indirectGeometryResult = vec3(0);
    vec3 indirectSpecularResult = traceCone(traceOrigin, reflectDir, specularAperture, false).xyz;
    if (smoothness >= uReflectionBlendLowerBound) // optimization for when the result is not used
        indirectGeometryResult = traceConeAgainstGeometry(traceOrigin, reflectDir, 0.1).xyz;
    float blendFactor = smoothstep(uReflectionBlendLowerBound, uReflectionBlendUpperBound, smoothness);
    vec3 indirectSpecular = mix(indirectSpecularResult.rgb, indirectGeometryResult.rgb * 1.5, blendFactor); // blends between specular highlight and reflection, bit of a hack

    // calculate resulting fragment
    vec3 diffuseGI = kD * albedo * indirectDiffuse;
    vec3 specularGI = F * indirectSpecular;     
    vec3 globalIllumination = (diffuseGI * uDiffuseBrightnessMultiplier) + specularGI;
    vec3 ambient = uAmbientColor * albedo * ambientOcclusion;
    vec3 finalColor = globalIllumination + ambient + indirectSpecular;
    FragColor = vec4(finalColor, 1.0);
}
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
uniform int uDebugIndex;
uniform int uVoxelRes;
uniform float uVoxelWorldSize;
uniform mat4 uViewMatrix; 

// tweakable settings
uniform float uConeAperture;
uniform float uStepMultiplier;
uniform float uMaxSteps;
uniform float uEmissiveThreshold;
uniform int   uNumDiffuseCones;
uniform float uDiffuseBrightnessMultiplier;
uniform float uOccludeThresholdForSecondaryCone;
uniform float uTransmittanceNeededForConeTermination;
uniform vec3  uAmbientColor; 

// set global vars
float CONE_APERTURE = uConeAperture;
float STEP_MULTIPLIER = uStepMultiplier;
float MAX_STEPS = uMaxSteps;
float EMISSIVE_THRESHOLD = uEmissiveThreshold;
float VOXEL_SIZE = uVoxelWorldSize / float(uVoxelRes);
int NUM_CONES = uNumDiffuseCones;
float BRIGHTNESS_MULTIPLIER = uDiffuseBrightnessMultiplier;
float OCCLUDE_THRESHOLD_FOR_SECONDARY_CONE = uOccludeThresholdForSecondaryCone;
float TRANSMITTANCE_NEEDED_CONE_TERMINATION = uTransmittanceNeededForConeTermination;
vec3 AMBIENT = uAmbientColor;
const float PI = 3.14159265359;
vec3 cameraPos = -vec3(
    dot(uViewMatrix[0].xyz, uViewMatrix[3].xyz),
    dot(uViewMatrix[1].xyz, uViewMatrix[3].xyz),
    dot(uViewMatrix[2].xyz, uViewMatrix[3].xyz)
);

vec3 worldToVoxel(vec3 pos) {
    return (pos / uVoxelWorldSize) + 0.5;
}

float calculateOcclusion(vec3 normal) {
    return length(normal);
}

// approximation for Fresnel
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// fresnel with roughness term for specular reflections
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
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

vec3 traceConeWithoutSecondary(vec3 origin, vec3 direction, float aperture) {
    vec3 accumulatedLight = vec3(0.0);
    float distance = VOXEL_SIZE * 2.0;
    float transmittance = 1.0;

    for (int i = 0; i < MAX_STEPS / 2; i++) {
        vec3 samplePos = origin + direction * distance;
        vec3 sampleCoord = worldToVoxel(samplePos);

        if (any(lessThan(sampleCoord, vec3(0.0))) || any(greaterThan(sampleCoord, vec3(1.0))))
            break;

        float coneDiameter = distance * aperture;
        float mipLevel = clamp(log2(coneDiameter / VOXEL_SIZE), 0.0, 6.0);

        vec4 voxelData2 = textureLod(voxelTex2, sampleCoord, mipLevel);
        vec4 voxelData1 = textureLod(voxelTex1, sampleCoord, mipLevel);

        float voxelEmissive = voxelData2.a;
        vec3 albedo = voxelData2.xyz;
        vec3 normal = voxelData1.xyz;
        float occlusion = calculateOcclusion(normal);

        // sample volume based on cone diameter
        float sampleVolume = coneDiameter * coneDiameter * coneDiameter;

        if (voxelEmissive > EMISSIVE_THRESHOLD) // surface is emissive
            accumulatedLight += albedo * voxelEmissive * transmittance * sampleVolume;

        // exponential attenuation based on occlusion
        float extinction = occlusion * STEP_MULTIPLIER;
        transmittance *= exp(-extinction);
        distance += coneDiameter * STEP_MULTIPLIER;

        if (transmittance < TRANSMITTANCE_NEEDED_CONE_TERMINATION)
            break;
    }

    return accumulatedLight + AMBIENT * transmittance;
}

vec3 traceCone(vec3 origin, vec3 direction, float aperture) {
    vec3 accumulatedLight = vec3(0.0);
    float distance = VOXEL_SIZE * 2.0;
    float transmittance = 1.0;

    for (int i = 0; i < MAX_STEPS; i++) {
        vec3 samplePos = origin + direction * distance;
        vec3 sampleCoord = worldToVoxel(samplePos);

        if (any(lessThan(sampleCoord, vec3(0.0))) || any(greaterThan(sampleCoord, vec3(1.0))))
            break;

        float coneDiameter = distance * aperture;
        float mipLevel = clamp(log2(coneDiameter / VOXEL_SIZE), 0.0, 6.0);

        // sample voxel data
        vec4 voxelData1 = textureLod(voxelTex1, sampleCoord, mipLevel);
        vec4 voxelData2 = textureLod(voxelTex2, sampleCoord, mipLevel);
        float voxelEmissive = voxelData2.a;
        float smoothness = voxelData1.a;
        vec3 albedo = voxelData2.xyz;
        vec3 normal = voxelData1.xyz;
        float currentOcclusion = calculateOcclusion(normal);
        float sampleVolume = coneDiameter * coneDiameter * coneDiameter;

        // calc lighting
        if (voxelEmissive > EMISSIVE_THRESHOLD) {
            // direct emissive contribution
            accumulatedLight += albedo * voxelEmissive * transmittance * sampleVolume;
        }
        else if (currentOcclusion > OCCLUDE_THRESHOLD_FOR_SECONDARY_CONE) {
            // secondary bounce for occluded surfaces
            vec3 secondaryPos = samplePos + normal * VOXEL_SIZE * 2.0;
            float roughness = 1.0 - smoothness;
            float secondaryAperture = tan(roughness * PI * 0.5);
            vec3 reflDir = normalize(reflect(-direction, normal));
            vec3 secondary = traceConeWithoutSecondary(secondaryPos, reflDir, secondaryAperture);

            // lambert diffuse
            float NdotL = max(dot(normal, -direction), 0.0);
            accumulatedLight += (albedo / PI) * secondary * NdotL * transmittance * currentOcclusion * sampleVolume;
        }
        else if (currentOcclusion > 0.01) {
            // partially occluded surfaces
            float NdotL = max(dot(normal, -direction), 0.0);
            vec3 indirectLight = albedo * currentOcclusion;
            accumulatedLight += (albedo / PI) * indirectLight * NdotL * transmittance * sampleVolume;
        }

        float extinction = currentOcclusion * STEP_MULTIPLIER; // exponential transmittance falloff
        transmittance *= exp(-extinction);

        distance += coneDiameter * STEP_MULTIPLIER; // adaptive step size based on cone diameter
        if (transmittance < TRANSMITTANCE_NEEDED_CONE_TERMINATION)
            break;
    }

    // add ambient contribution scaled by remaining transmittance
    return accumulatedLight + AMBIENT * transmittance;
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
        float angle = float(i) * 2.0 * PI / float(NUM_CONES);
        // cosine weighted sampling - more samples near normal
        float phi = (float(i) + 0.5) / float(NUM_CONES);
        float cosTheta = sqrt(1.0 - phi);
        float sinTheta = sqrt(phi);

        vec3 direction = normalize(
            tangent * (cos(angle) * sinTheta) +
            bitangent * (sin(angle) * sinTheta) +
            normal * cosTheta
        );

        vec3 coneLight = traceCone(origin, direction, CONE_APERTURE);
        totalLight += coneLight * cosTheta; // weight by cosine term
    }
    return totalLight * (2.0 * PI / float(NUM_CONES));
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

    if (length(worldNormal) < 0.1) { // fragment is the sky
        FragColor = vec4(albedo, 1.0);
        return;
    }

    // check if surface is emissive, if so, set fragment color to it
    if (emissiveFactor > EMISSIVE_THRESHOLD) {
        FragColor = vec4(emissiveRgb * emissiveFactor, 1.0);
        return;
    }
    worldNormal = normalize(worldNormal);
    vec3 traceOrigin = worldPos + worldNormal * VOXEL_SIZE * 2.0;
    vec3 viewDir = normalize(cameraPos - worldPos);
    float NdotV = max(dot(worldNormal, viewDir), 0.0);
    vec3 F0 = mix(vec3(0.04), albedo, metallic);  // calculate base reflectivity
    float roughness = 1.0 - smoothness;
    vec3 F = fresnelSchlickRoughness(NdotV, F0, roughness);     // energy conservation, diffuse is reduced by specular
    vec3 kD = (1.0 - F) * (1.0 - metallic); // mfetals have no diffuse
    vec3 indirectDiffuse = sampleHemisphere(traceOrigin, worldNormal) * BRIGHTNESS_MULTIPLIER;  // indirect diffuse lighting via hemisphere sampling
    vec3 reflectDir = reflect(-viewDir, worldNormal);     // specular reflection
    float specularAperture = tan(roughness * PI * 0.5);
    vec3 specularLight = traceCone(traceOrigin, reflectDir, specularAperture) * BRIGHTNESS_MULTIPLIER;

    // combine lighting with physically based energy conservation
    vec3 diffuse = kD * albedo * indirectDiffuse / PI;
    vec3 specular = specularLight * F;

    vec3 finalColor = diffuse + specular;

    FragColor = vec4(finalColor, 1.0);
}
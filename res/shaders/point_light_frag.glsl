#version 440

layout(binding = 0, rgba8) uniform image3D voxelTex0; // Pos.xyz + Metallic
layout(binding = 1, rgba8) uniform image3D voxelTex1; // Normal.xyz + Smoothness
layout(binding = 2, rgba8) uniform image3D voxelTex2; // Albedo.rgb + EmissiveFactor

uniform int   uVoxelRes;
uniform float uVoxelWorldSize;
uniform int   uRenderMode; // 0 = voxelize, 1 = gbuffer
uniform vec3 uVoxelCenter;

layout(location = 0) out vec4 gPosition; // world position.xyz + metallic
layout(location = 1) out vec4 gNormal;   // world normal.xyz + smoothness
layout(location = 2) out vec4 gAlbedo;   // albedo.rgb + emissiveFactor
layout(location = 3) out vec4 gEmissive; // emissive.rgb + spare channel

// additional uniforms
uniform vec3 uLightColor;

in vec3 worldPos;
in vec3 normal;

struct MaterialData {
    vec3 pos, nrm, alb, emi; // world position, world normal, albedo, emissive color
    float mtl, smoothness, emiFac; // metalic, smoothness, emissive factor (strength of emission)
};

void writeRenderInfo(MaterialData m) {
    if (uRenderMode == 0) { // voxel
        // center the voxel grid around uVoxelCenter
        vec3 vpos = (m.pos - uVoxelCenter + uVoxelWorldSize * 0.5) / uVoxelWorldSize;

        if (any(lessThan(vpos, vec3(0))) || any(greaterThan(vpos, vec3(1)))) return;
        ivec3 texCoord = ivec3(vpos * float(uVoxelRes - 1));

        int splatRadius = 4;
        for (int x = -splatRadius; x <= splatRadius; ++x) {
            for (int y = -splatRadius; y <= splatRadius; ++y) {
                for (int z = -splatRadius; z <= splatRadius; ++z) {
                    ivec3 tc = texCoord + ivec3(x, y, z);
                    if (any(lessThan(tc, ivec3(0))) || any(greaterThanEqual(tc, ivec3(uVoxelRes))))
                        continue;

                    imageStore(voxelTex0, tc, vec4(vpos, m.mtl));
                    imageStore(voxelTex1, tc, vec4(normalize(m.nrm), m.smoothness));
                    imageStore(voxelTex2, tc, vec4(m.alb, m.emiFac));
                }
            }
        }
    }
    else { // gbuffer
        gPosition = vec4(m.pos, m.mtl);
        gNormal = vec4(normalize(m.nrm), m.smoothness);
        gAlbedo = vec4(m.alb, m.emiFac);
        gEmissive = vec4(m.emi * m.emiFac, 0);
    }
}
void main() {
    MaterialData m;
    m.pos = worldPos;
    m.nrm = normal;
    m.alb = vec3(uLightColor);
    m.emi = vec3(uLightColor);
    m.mtl = 0.0;
    m.smoothness = 0.0;
    m.emiFac = 1;

    writeRenderInfo(m);
}

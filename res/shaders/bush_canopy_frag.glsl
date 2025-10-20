#version 440

layout(binding = 0, rgba16f) uniform image3D voxelTex0; // Pos.xyz + Metallic
layout(binding = 1, rgba16f) uniform image3D voxelTex1; // Normal.xyz + Smoothness
layout(binding = 2, rgba16f) uniform image3D voxelTex2; // Albedo.rgb + EmissiveFactor

uniform int   uVoxelRes;
uniform float uVoxelWorldSize;
uniform int   uRenderMode; // 0 = voxelize, 1 = gbuffer
uniform vec3 uVoxelCenter;
uniform int uVoxelSplatRadius;

layout(location = 0) out vec4 gPosition; // world position.xyz + metallic
layout(location = 1) out vec4 gNormal;   // world normal.xyz + smoothness
layout(location = 2) out vec4 gAlbedo;   // albedo.rgb + emissiveFactor
layout(location = 3) out vec4 gEmissive; // emissive.rgb + spare channel

in vec3 worldPos;
in vec3 normal; // must be world space
in vec2 uvCoord;
out vec4 fragColor;

uniform sampler2D colourTexture;
uniform sampler2D normalTexture;

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

        int splatRadius = uVoxelSplatRadius;
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
    m.alb = texture2D(colourTexture, uvCoord).rgb;
    m.emi = vec3(0.0);		
    m.mtl = 0.0;		// 0 for non-metalic surfaces
    m.smoothness = 0.15;	// can be thought of as shinyness, eg concrete has a low value
    m.emiFac = 0.0;		// either 0 or >= 1

    writeRenderInfo(m);
    fragColor = vec4(1); // Optional debug color
}

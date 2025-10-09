#version 440

layout(binding = 0, rgba16f) uniform image3D voxelTex0; // Pos.xyz + Metallic
layout(binding = 1, rgba16f) uniform image3D voxelTex1; // Normal.xyz + Smoothness
layout(binding = 2, rgba16f) uniform image3D voxelTex2; // Albedo.rgb + EmissiveFactor

uniform int   uVoxelRes;
uniform float uVoxelWorldSize;
uniform int   uRenderMode; // 0 = voxelize, 1 = gbuffer
uniform vec3 uVoxelCenter;

layout(location = 0) out vec4 gPosition; // world position.xyz + metallic
layout(location = 1) out vec4 gNormal;   // world normal.xyz + smoothness
layout(location = 2) out vec4 gAlbedo;   // albedo.rgb + emissiveFactor
layout(location = 3) out vec4 gEmissive; // emissive.rgb + spare channel

out vec4 fragColor;

struct MaterialData {
	vec3 pos, nrm, alb, emi; // world position, world normal, albedo, emissive color
	float mtl, smoothness, emiFac; // metalic, smoothness, emissive factor (strength of emission)
};

in VertexData {
	vec3 worldPos;
	vec3 normal;
	vec2 textureCoord;
} f_in;

uniform sampler2D water_texture;
uniform sampler2D water_normal_texture;
uniform sampler2D water_dudv_texture;
const float TEXTURE_SCALAR = 8.0f; // Don't just want the single texture for entire thing so repreat
const float WAVE_STRENGTH = 0.005;
uniform float move_factor;

uniform float metallic;
uniform float smoothness;

void writeRenderInfo(MaterialData m) {
    if (uRenderMode == 0) { // voxel
        // center the voxel grid around uVoxelCenter
        vec3 vpos = (m.pos - uVoxelCenter + uVoxelWorldSize * 0.5) / uVoxelWorldSize;

        if (any(lessThan(vpos, vec3(0))) || any(greaterThan(vpos, vec3(1)))) return;
        ivec3 texCoord = ivec3(vpos * float(uVoxelRes - 1));

        int splatRadius = 1;
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

	vec2 wavecoords = vec2(f_in.textureCoord.x + move_factor, f_in.textureCoord.y + move_factor * 0.8);
	vec2 distortion = (texture(water_dudv_texture, wavecoords * 2.0).rg * 2.0 - 1.0) * WAVE_STRENGTH;
	vec2 distorted_coords = f_in.textureCoord + distortion;

	vec3 tex_col = texture(water_texture, distorted_coords * TEXTURE_SCALAR).rgb;
	
	vec3 norm = texture(water_normal_texture, distorted_coords * 12.0).rgb;
	norm = vec3(norm.x, norm.z, norm.y);
	norm = normalize(norm * 2.0 - 1.0);
	
	m.pos = f_in.worldPos;
	m.nrm = norm;
	m.alb = tex_col;
	m.emi = vec3(0.0);
	m.mtl = metallic;
	m.smoothness = smoothness;
	m.emiFac = 0.0;

	writeRenderInfo(m);
	fragColor = vec4(1); // Optional debug color
}

#version 440

// Voxel stuff
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

struct MaterialData {
	vec3 pos, nrm, alb, emi; // world position, world normal, albedo, emissive color
	float mtl, smoothness, emiFac; // metalic, smoothness, emissive factor (strength of emission)
};

// regular stuff

uniform mat4 uProjectionMatrix;
uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform vec3 uColor;

// viewspace data
in VertexData {
	vec3 position;
	vec3 normal;
	vec2 textureCoord;
} f_in;

// framebuffer output
out vec4 fb_color;

uniform sampler2D heightMap;

uniform sampler2D water_texture;
uniform sampler2D sand_texture;
uniform sampler2D grass_texture;
uniform sampler2D rock_texture;
uniform sampler2D snow_texture;
uniform bool useTexturing; // whether or not to use texturing or just display the heightmap
uniform bool useFakedLighting; // whether to use faked lighting, just until proper lighting is implemented

// Slope texturing parameters
uniform float min_rock_slope;
uniform float max_grass_slope;

uniform float tex_base_scalar;
uniform float triplanar_sharpness; // Controls blend sharpness between projections
uniform bool use_triplanar_mapping;

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

// Original height-based implementation of texturing, not the best but leaving in for future reference
vec3 getTerrainColor(vec2 uv, float height) {
	// Define height thresholds
	const float water_level = 0.15;
	const float sand_level = 0.25;
	const float grass_level = 0.5;
	const float rock_level = 0.65;

	// Define blend ranges for smooth transitions
	const float blend_range = 0.25;

	// Sample all textures
	vec3 water_color = texture(water_texture, uv).rgb;
	vec3 sand_color = texture(sand_texture, uv).rgb;
	vec3 grass_color = texture(grass_texture, uv).rgb;
	vec3 rock_color = texture(rock_texture, uv).rgb;
	vec3 snow_color = texture(snow_texture, uv).rgb;

	vec3 final_color;

	// Water to sand transition
	if (height < sand_level) {
		float t = smoothstep(water_level - blend_range, water_level + blend_range, height);
		final_color = mix(water_color, sand_color, t);
	}
	// Sand to grass transition
	else if (height < grass_level) {
		float t = smoothstep(sand_level - blend_range, sand_level + blend_range, height);
		final_color = mix(sand_color, grass_color, t);
	}
	// Grass to rock transition
	else if (height < rock_level) {
		float t = smoothstep(grass_level - blend_range, grass_level + blend_range, height);
		final_color = mix(grass_color, rock_color, t);
	}
	// Rock to snow transition
	else {
		float t = smoothstep(rock_level - blend_range, rock_level + blend_range, height); // NOTE - tweak this a bit cuz not the best transition
		final_color = mix(rock_color, snow_color, t);
	}

	return final_color;
}

vec3 triplanarSample(sampler2D tex, vec3 worldPos, vec3 normal) {
	// Calculate blend weights based on normal
	vec3 blendWeights = abs(normal);
	blendWeights = pow(blendWeights, vec3(triplanar_sharpness));
	blendWeights /= (blendWeights.x + blendWeights.y + blendWeights.z);

	// Sample texture from each projection plane
	vec3 xProjection = texture(tex, worldPos.zy).rgb;
	vec3 yProjection = texture(tex, worldPos.xz).rgb;
	vec3 zProjection = texture(tex, worldPos.xy).rgb;

	// Blend the three projections
	return xProjection * blendWeights.x +
		yProjection * blendWeights.y +
		zProjection * blendWeights.z;
}

// Get slope texture with triplanar mapping
vec3 getTerrainColorSlopeTriplanar(vec3 worldPos, vec3 normal) {
	vec3 grass_col = triplanarSample(grass_texture, worldPos, normal);
	vec3 rock_col = triplanarSample(rock_texture, worldPos, normal);

	float rock_grass_weight = normal.y;
	
	rock_grass_weight = max(min_rock_slope, rock_grass_weight);
	rock_grass_weight = min(max_grass_slope, rock_grass_weight);
	rock_grass_weight -= min_rock_slope;
	rock_grass_weight /= max_grass_slope - min_rock_slope;

	vec3 col = mix(rock_col, grass_col, rock_grass_weight);
	return col;
}

// Get slope texture without triplanar mapping
vec3 getTerrainColorSlope(vec2 uv, vec3 normal) {
	vec3 grass_col = texture(grass_texture, uv).rgb;
	vec3 rock_col = texture(rock_texture, uv).rgb;

	float rock_grass_weight = normal.y;
	
	rock_grass_weight = max(min_rock_slope, rock_grass_weight);
	rock_grass_weight = min(max_grass_slope, rock_grass_weight);
	rock_grass_weight -= min_rock_slope;
	rock_grass_weight /= max_grass_slope - min_rock_slope; // TODO - potential divide by zero

	vec3 col = mix(rock_col, grass_col, rock_grass_weight);
	return col;
}

void main() {
	MaterialData m;
	float height = texture(heightMap, f_in.textureCoord).r;
	vec3 col;
	if (useTexturing) {
		col = (use_triplanar_mapping) ? getTerrainColorSlopeTriplanar(f_in.position, normalize(f_in.normal)) : getTerrainColorSlope(f_in.textureCoord * tex_base_scalar, f_in.normal);
	} else {
		col = vec3(height, height, height);
	}

	if (useFakedLighting) {
		col = mix(vec3(height, height, height), col, 0.8);

		// Calculate lighting
		vec3 eye = normalize(vec3(0.0, 10.0, 0.0)); // faked light position
		vec3 normal = normalize(f_in.normal);

		// Diffuse lighting
		float diffuse = max(dot(normal, eye), 0.0);

		// Ambient + diffuse
		float ambient = 0.1;
		float light = ambient + diffuse;

		// Apply lighting
		col = col * light;

	}

	m.pos = f_in.position;
	m.nrm = f_in.normal;
	m.alb = col;
	m.emi = vec3(0.0);
	m.mtl = 0.0; // TODO - tweak these later, probs based on texture
	m.smoothness = 0.0;
	m.emiFac = 0.0;

	writeRenderInfo(m);
	fb_color = vec4(1.0);
}

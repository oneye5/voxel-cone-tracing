#version 400 core

uniform mat4 uProjectionMatrix;
uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform vec3 uColor;

// Mesh stuff
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out VertexData {
	vec3 position;
	vec3 normal;
	vec2 textureCoord;
} v_out;

uniform float max_height; // TODO - not really needed, might remove later cuz amplitude is better
uniform float amplitude;
uniform sampler2D heightMap; // The heightmap

uniform int subdivisions; // How many subdivisions the plane has (needed for normal calculation)
uniform float terrain_size_scalar; // The amount the plane gets scaled up by with the model matrix (needed for normal calculation)
// TODO - separate into vec2 if allowing non square terrain

uniform bool draw_from_min; // whether or not to draw from min height
uniform float min_height;

// Calculate the normals for a position by sampling neighbouring points
vec3 calculateNormal(vec2 texCoord) {
	vec2 heightMapSize = textureSize(heightMap, 0);
	vec2 texelSize = 1.0f / heightMapSize;

	// Sample neighboring heights
	//float heightscale = amplitude * max_height;
	float heightscale = amplitude;
	float heightL = texture(heightMap, texCoord + vec2(-texelSize.x, 0.0)).r * heightscale;
	float heightR = texture(heightMap, texCoord + vec2(texelSize.x, 0.0)).r * heightscale;
	float heightD = texture(heightMap, texCoord + vec2(0.0, -texelSize.y)).r * heightscale;
	float heightU = texture(heightMap, texCoord + vec2(0.0, texelSize.y)).r * heightscale;

	float modelSpacing = 1.0 / float(subdivisions); // Distance between vertices in model space (0 to 1)
	float worldSpacing = modelSpacing * terrain_size_scalar;  // After model matrix scale

	// tangent vectors
	vec3 tangentX = vec3(worldSpacing * 2.0, heightR - heightL, 0.0);
	vec3 tangentZ = vec3(0.0, heightU - heightD, worldSpacing * 2.0);

	vec3 normal = normalize(cross(tangentZ, tangentX));
	return normal;
}


void main() {
	// transform vertex data to viewspace
	float height = texture(heightMap, aTexCoord).r * amplitude;
	float y_pos = (draw_from_min) ? height - min_height : height;

	// Push edge vertices down to y=0.0 to prevent the open ended sides (not the best implementation but whatev)
	bool isEdgeVertex = (aTexCoord.x <= 0.001 || aTexCoord.x >= 0.999 ||
						 aTexCoord.y <= 0.001 || aTexCoord.y >= 0.999);
	if (isEdgeVertex) {
		y_pos = 0.0; // Push way down
	}

	vec3 pos = vec3(aPosition.x, y_pos, aPosition.z);
	v_out.position = (uModelMatrix * vec4(pos, 1.0)).xyz; // Use world normal
	mat4 modelView = uViewMatrix * uModelMatrix;

	// calcualte world space normal
	vec3 world_normal = calculateNormal(aTexCoord);
	if (isEdgeVertex) {
		// Kinda evil method but it saves me like 4 extra if statements
	    vec2 centered_coord = (aTexCoord - 0.5) * 2.0;
    
		if (abs(centered_coord.x) > abs(centered_coord.y)) {
			world_normal = vec3(sign(centered_coord.x), 0.0, 0.0);
		} else {
			world_normal = vec3(0.0, 0.0, sign(centered_coord.y));
		}
	}
	mat3 normalMatrix = transpose(inverse(mat3(uModelMatrix)));
	v_out.normal = normalize(normalMatrix * world_normal);
	v_out.textureCoord = aTexCoord;

	// set the screenspace position (needed for converting to fragment data)
	gl_Position = uProjectionMatrix * modelView * vec4(pos, 1);
}

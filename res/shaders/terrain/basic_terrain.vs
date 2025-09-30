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

uniform float max_height;
uniform float min_height;

uniform sampler2D heightMap; // The heightmap


// TODO - lowkey kinda vibecoded so reimplement but better later
vec3 calculateNormal(vec2 texCoord) {
	// Get heightmap dimensions using textureSize
	ivec2 heightMapSize = textureSize(heightMap, 0);
	
	// Calculate texel size
	float texelSizeX = 1.0 / float(heightMapSize.x);
	float texelSizeY = 1.0 / float(heightMapSize.y);
	
	// Sample neighboring heights
	float heightL = texture(heightMap, texCoord + vec2(-texelSizeX, 0.0)).r * max_height;
	float heightR = texture(heightMap, texCoord + vec2(texelSizeX, 0.0)).r * max_height;
	float heightD = texture(heightMap, texCoord + vec2(0.0, -texelSizeY)).r * max_height;
	float heightU = texture(heightMap, texCoord + vec2(0.0, texelSizeY)).r * max_height;
	
	// Calculate tangent vectors
	// Since your terrain goes from 0 to 1 in world space, the step size is texelSizeX and texelSizeY
	vec3 tangentX = vec3(2.0 * texelSizeX, heightR - heightL, 0.0);	 // 2.0 because we're sampling both sides
	vec3 tangentZ = vec3(0.0, heightU - heightD, 2.0 * texelSizeY);
	
	// Cross product to get normal
	vec3 normal = normalize(cross(tangentX, tangentZ));
	return normal;
}


void main() {
	// transform vertex data to viewspace
	float height = texture(heightMap, aTexCoord).r;
	float y_pos = max_height * height;
	y_pos = clamp(y_pos, min_height, max_height);

	vec3 pos = vec3(aPosition.x, y_pos, aPosition.z);
	vec3 world_normal = calculateNormal(aTexCoord);
	
	v_out.position = (uModelMatrix * vec4(pos, 1.0)).xyz; // Use world normal
	
	mat4 modelView = uViewMatrix * uModelMatrix;
	v_out.normal = normalize((modelView * vec4(world_normal, 0)).xyz);
	v_out.textureCoord = aTexCoord;

	// set the screenspace position (needed for converting to fragment data)
	gl_Position = uProjectionMatrix * modelView * vec4(pos, 1);
}

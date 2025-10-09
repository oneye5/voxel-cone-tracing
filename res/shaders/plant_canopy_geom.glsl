#version 440

layout (points) in;
layout (triangle_strip, max_vertices = 5) out;

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;
uniform int uVoxelRes;
uniform float uVoxelWorldSize;
uniform int uRenderMode; // 0 = write voxels, 1 = write to gbuffer

in vec3 inWorldPos[];
in vec3 inNormal[];
out vec3 worldPos;
out vec3 normal;


// uniform mat4 projection;
// uniform mat4 view;
// uniform mat4 model;

// Cylinder radius
uniform float lineRadius = 0.1;

// // Input vertices from vertex shader
// in vec3 vertexColor[];
// // Output for fragment shader
// out vec3 fragColor;

void main() {
	// Get start and end points of the line
	vec4 pt = gl_in[0].gl_Position;
	mat4 mvp = uProjectionMatrix * uViewMatrix * uModelMatrix;

	vec3 right = normalize(cross(inNormal[0], vec3(0,1,0)));
	vec3 up = normalize(cross(right, inNormal[0]));

	// Top circle
	gl_Position = mvp * vec4(pt.xyz, 1.0);
	worldPos = inWorldPos[0];
	normal = vec3(0,1,0);
	EmitVertex();
	float sc = 0.05;

	gl_Position = mvp * vec4(pt.xyz - right * sc + up * sc, 1.0);
	worldPos = inWorldPos[0];
	normal = vec3(0,1,0);
	EmitVertex();

	gl_Position = mvp * vec4(pt.xyz + right * sc + up * sc, 1.0);
	worldPos = inWorldPos[0];
	normal = vec3(0,1,0);
	EmitVertex();

	gl_Position = mvp * vec4(pt.xyz - right * sc + up * sc * 2, 1.0);
	worldPos = inWorldPos[0];
	normal = vec3(0,1,0);
	EmitVertex();

	gl_Position = mvp * vec4(pt.xyz + right * sc + up * sc * 3, 1.0);
	worldPos = inWorldPos[0];
	normal = vec3(0,1,0);
	EmitVertex();

	// Close the triangle strip
	EndPrimitive();
}

#version 440

layout (points) in;
layout (triangle_strip, max_vertices = 6) out;

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
out vec2 uvCoord;


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
	vec3 norm = normalize(-cross(right, up));
	float sc = 0.25;
	float dx = 0.0;
	float dy = 0.0;

	gl_Position = mvp * vec4(pt.xyz + right * sc * dx + up * sc * dy, 1.0); // 0
	uvCoord = vec2(0.4340277777777778, 0.8796296296296297);
	worldPos = inWorldPos[0];
	normal = norm;
	EmitVertex();

	dx = -0.08796296296296297; dy = 0.10763888888888895;
	gl_Position = mvp * vec4(pt.xyz + right * sc * dx + up * sc * dy, 1.0); // 5
	uvCoord = vec2(0.3460648148148148, 0.7719907407407407);
	worldPos = inWorldPos[0];
	normal = norm;
	EmitVertex();

	dx = 0.15625; dy = 0.12037037037037035;
	gl_Position = mvp * vec4(pt.xyz + right * sc * dx + up * sc * dy, 1.0); // 1
	uvCoord = vec2(0.5902777777777778, 0.7592592592592593);
	worldPos = inWorldPos[0];
	normal = norm;
	EmitVertex();

	dx = -0.10532407407407407; dy = 0.3784722222222222;
	gl_Position = mvp * vec4(pt.xyz + right * sc * dx + up * sc * dy, 1.0); // 4
	uvCoord = vec2(0.3287037037037037, 0.5011574074074074);
	worldPos = inWorldPos[0];
	normal = norm;
	EmitVertex();

	dx = 0.14120370370370372; dy = 0.4502314814814815;
	gl_Position = mvp * vec4(pt.xyz + right * sc * dx + up * sc * dy, 1.0); // 2
	uvCoord = vec2(0.5752314814814815, 0.42939814814814814);
	worldPos = inWorldPos[0];
	normal = norm;
	EmitVertex();

	dx = 0.00694444444444442; dy = 0.7337962962962963;
	gl_Position = mvp * vec4(pt.xyz + right * sc * dx + up * sc * dy, 1.0); // 3
	uvCoord = vec2(0.4409722222222222, 0.14583333333333334);
	worldPos = inWorldPos[0];
	normal = norm;
	EmitVertex();

	// Close the triangle strip
	EndPrimitive();
}

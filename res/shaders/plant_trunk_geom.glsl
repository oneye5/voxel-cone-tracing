#version 440

layout (lines) in;
layout (triangle_strip, max_vertices = 24) out;

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;
uniform int uVoxelRes;
uniform float uVoxelWorldSize;
uniform int uRenderMode; // 0 = write voxels, 1 = write to gbuffer

in vec3 inWorldPos[];
in vec3 inNormal[];
in float sizes[];
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
    vec4 start = gl_in[0].gl_Position;
    vec4 end = gl_in[1].gl_Position;

    // Calculate line direction and length
    vec3 lineDir = normalize(end.xyz - start.xyz);
    float lineLength = distance(start.xyz, end.xyz);

    // Create a perpendicular vector (for cylinder cross-section)
    vec3 up = abs(lineDir.y) > 0.9 ? vec3(1, 0, 0) : vec3(0, 1, 0);
    vec3 right = normalize(cross(lineDir, up));
    up = normalize(cross(right, lineDir));
	float decay = 0.5;
	float startMult = pow(decay, sizes[0]);
	float endMult = pow(decay, sizes[1]);

	int uvpos = 0;
    // Generate cylinder vertices
    for (int i = 0; i <= 8; i++) {
        float angle = i * (2.0 * 3.14159 / 8.0);
        vec3 offset = lineRadius * (right * cos(angle) + up * sin(angle));

        // Bottom circle
        gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4(start.xyz + (offset * startMult), 1.0);
		uvCoord = vec2((angle / 6.283185308)/8, 0);
		worldPos = inWorldPos[0];
		normal = normalize(offset);
        EmitVertex();
	
	uvpos += 1;

        // Top circle
        gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4(end.xyz + (offset * endMult), 1.0);
		worldPos = inWorldPos[1];
		uvCoord = vec2((angle / 6.283185308)/8, 1);
		normal = normalize(offset);
        EmitVertex();
    }

    // Close the triangle strip
    EndPrimitive();
}

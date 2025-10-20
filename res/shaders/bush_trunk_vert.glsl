#version 440

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout (location = 4) in float inSize;

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;
uniform int uVoxelRes;
uniform float uVoxelWorldSize;
uniform int uRenderMode; // 0 = write voxels, 1 = write to gbuffer

out float sizes;
out vec3 inNormal;
out vec3 inWorldPos;
out vec3 realInside;

void main() {
	sizes =  inSize;
    inWorldPos = (uModelMatrix * vec4(aPosition, 1.0)).xyz;
    mat3 normalMatrix = transpose(inverse(mat3(uModelMatrix)));
    inNormal = normalize(normalMatrix * aNormal); 

    gl_Position = /* uProjectionMatrix * uViewMatrix * uModelMatrix */ vec4(aPosition, 1.0);
}

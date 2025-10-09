#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <stdexcept>
#include <functional>
#include "gBufferPrepass.hpp"
#include <cgra/cgra_shader.hpp>
#include <iostream>
#include "voxelizer.hpp"

class gBufferLightingPass {
public:
	struct light_pass_params {
		float uConeAperture;
		float uStepMultiplier;
		float uMaxSteps;
		float uEmissiveThreshold;
		int   uNumDiffuseCones;
		float uDiffuseBrightnessMultiplier;
		float uOccludeThresholdForSecondaryCone;
		float uTransmittanceNeededForConeTermination;
		glm::vec3  uAmbientColor;
		float uSecondaryConeMaxStepMultiplier;
		float uReflectionBlendLowerBound;
		float uReflectionBlendUpperBound;
		glm::vec3 uHorizonColor;
		glm::vec3 uZenithColor;
		bool uToneMapEnable;
		float uReflectionAperture;
	};
	light_pass_params params;

	gBufferLightingPass(gBufferPrepass* prepassObj, Voxelizer* voxelizerObj) {
		prepass = prepassObj; 
		voxelizer = voxelizerObj;

		cgra::shader_builder sb;
		sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//fullscreen_quad_vert.glsl"));
		sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//lighting_pass_frag.glsl"));
		shader = sb.build();

		setupQuad();

		glUseProgram(shader);
		glUniform1i(glGetUniformLocation(shader, "voxelTex0"), 4);
		glUniform1i(glGetUniformLocation(shader, "voxelTex1"), 5);
		glUniform1i(glGetUniformLocation(shader, "voxelTex2"), 6);

		// set default params
		params.uConeAperture = 1.0;
		params.uStepMultiplier = 0.6;
		params.uMaxSteps = 256;
		params.uEmissiveThreshold = 0.0;
		params.uNumDiffuseCones = 32;
		params.uDiffuseBrightnessMultiplier = 20000.0;
		params.uOccludeThresholdForSecondaryCone = 0.8; // TODO unused
		params.uTransmittanceNeededForConeTermination = 0.01;
		params.uAmbientColor = glm::vec3(0.2);
		params.uSecondaryConeMaxStepMultiplier = 0.25; // TODO unused
		params.uReflectionBlendLowerBound = 0.75;
		params.uReflectionBlendUpperBound = 1;
		params.uHorizonColor = glm::vec3(0.5, 0.8, 0.9);
		params.uZenithColor = glm::vec3(0.2, 0.4, 0.8);
		params.uToneMapEnable = true;
		params.uReflectionAperture = 0.05;
	}

	~gBufferLightingPass() {
		glUseProgram(0);
		if (shader != 0 && glIsProgram(shader)) {
			glDeleteProgram(shader);
			shader = 0;
		}
		if (quadVBO != 0) {
			glDeleteBuffers(1, &quadVBO);
			quadVBO = 0;
		}
		if (quadVAO != 0) {
			glDeleteVertexArrays(1, &quadVAO);
			quadVAO = 0;
		}
	}

	void runPass(glm::mat4& view, int debugMode = 0) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);  // Render to screen
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(shader);

		glUniformMatrix4fv(glGetUniformLocation(shader, "uViewMatrix"), 1, GL_FALSE, glm::value_ptr(view));
		glUniform1f(glGetUniformLocation(shader, "uConeAperture"), params.uConeAperture);
		glUniform1f(glGetUniformLocation(shader, "VOXEL_SIZE"), voxelizer->m_params.worldSize/float(voxelizer->m_params.resolution));
		glUniform1f(glGetUniformLocation(shader, "uStepMultiplier"), params.uStepMultiplier);
		glUniform1f(glGetUniformLocation(shader, "uMaxSteps"), params.uMaxSteps);
		glUniform1f(glGetUniformLocation(shader, "uEmissiveThreshold"), params.uEmissiveThreshold);
		glUniform1f(glGetUniformLocation(shader, "uDiffuseBrightnessMultiplier"), params.uDiffuseBrightnessMultiplier);
		glUniform1f(glGetUniformLocation(shader, "uOccludeThresholdForSecondaryCone"), params.uOccludeThresholdForSecondaryCone);
		glUniform1f(glGetUniformLocation(shader, "uTransmittanceNeededForConeTermination"), params.uTransmittanceNeededForConeTermination);
		glUniform1f(glGetUniformLocation(shader, "uSecondaryConeMaxStepMultiplier"), params.uSecondaryConeMaxStepMultiplier);
		glUniform1i(glGetUniformLocation(shader, "uNumDiffuseCones"), params.uNumDiffuseCones);
		glUniform3fv(glGetUniformLocation(shader, "uAmbientColor"), 1, glm::value_ptr(params.uAmbientColor));
		glUniform3fv(glGetUniformLocation(shader, "uVoxelCenter"), 1, value_ptr(voxelizer->m_params.center));
		glUniform1f(glGetUniformLocation(shader, "uReflectionBlendLowerBound"), params.uReflectionBlendLowerBound);
		glUniform1f(glGetUniformLocation(shader, "uReflectionBlendUpperBound"), params.uReflectionBlendUpperBound);
		glUniform3fv(glGetUniformLocation(shader, "uHorizonColor"), 1, value_ptr(params.uHorizonColor));
		glUniform3fv(glGetUniformLocation(shader, "uZenithColor"), 1, value_ptr(params.uZenithColor));
		glUniform1i(glGetUniformLocation(shader, "uToneMapEnable"), params.uToneMapEnable);
		glUniform1f(glGetUniformLocation(shader, "uReflectionAperture"), params.uReflectionAperture);



		auto invView = glm::inverse(view);
		auto camPos = glm::vec3(invView[3]);
		glUniform3fv(glGetUniformLocation(shader, "cameraPos"), 1, glm::value_ptr(camPos));
		float mip = (float)voxelizer->m_params.mipLevels;
		glUniform1f(glGetUniformLocation(shader, "uMipLevelCount"), mip);

		// Bind debug mode
		glUniform1i(glGetUniformLocation(shader, "uDebugIndex"), debugMode);

		// Bind all G-buffer attachments
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, prepass->getAttachment(0)); // Position
		glUniform1i(glGetUniformLocation(shader, "gBufferPosition"), 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, prepass->getAttachment(1)); // Normal
		glUniform1i(glGetUniformLocation(shader, "gBufferNormal"), 1);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, prepass->getAttachment(2)); // Albedo
		glUniform1i(glGetUniformLocation(shader, "gBufferAlbedo"), 2);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, prepass->getAttachment(3)); // Emissive
		glUniform1i(glGetUniformLocation(shader, "gBufferEmissive"), 3);

		// voxels
		glUniform1i(glGetUniformLocation(shader, "uVoxelRes"), voxelizer->m_params.resolution);
		glUniform1f(glGetUniformLocation(shader, "uVoxelWorldSize"), voxelizer->m_params.worldSize);

		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_3D, voxelizer->m_voxelTex0);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_3D, voxelizer->m_voxelTex1);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_3D, voxelizer->m_voxelTex2); // no uniform setting needed, already done in constructor

		// Draw fullscreen quad
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
private:
	Voxelizer* voxelizer;
	gBufferPrepass* prepass;
	GLuint shader; 
	GLuint quadVAO;
	GLuint quadVBO;
	void setupQuad() {
		float quadVertices[] = {
			// positions   // texCoords
			-1.0f,  1.0f,  0.0f, 1.0f,
			-1.0f, -1.0f,  0.0f, 0.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,

			-1.0f,  1.0f,  0.0f, 1.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,
			 1.0f,  1.0f,  1.0f, 1.0f
		};

		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);

		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

		// Position attribute
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		// Texture coordinate attribute  
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
		glEnableVertexAttribArray(1);
	}
};

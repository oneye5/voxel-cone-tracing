#pragma once
#include "renderable.hpp"
#include "cgra/cgra_wavefront.hpp"
#include "cgra/cgra_shader.hpp"
#include <glm/gtc/type_ptr.hpp>
#include "cgra/cgra_mesh.hpp"
#include <glm/gtc/matrix_transform.hpp>

class PointLightRenderable : public Renderable {
public:
    PointLightRenderable() {
        // Build shader
        cgra::shader_builder sb;
        sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//example_vct_compatible_vert.glsl"));
        sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//point_light_frag.glsl"));
        shader = sb.build();

        // Load mesh
        mesh = cgra::load_wavefront_data(CGRA_SRCDIR + std::string("//res//assets//ball2.obj")).build();

        // Default transform & color
        modelTransform = glm::mat4(1.0f);
        lightColor = glm::vec3(1,0.85,0.8);
    }

    GLuint getShader() override { return shader; }

    void setProjViewUniforms(const glm::mat4& view, const glm::mat4& proj) const override {
        glUseProgram(shader);

        glm::mat4 modelview = view * modelTransform;

        glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, GL_FALSE, glm::value_ptr(modelview));
        glUniformMatrix4fv(glGetUniformLocation(shader, "uViewMatrix"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shader, "uModelMatrix"), 1, GL_FALSE, glm::value_ptr(modelTransform));
        glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1, GL_FALSE, glm::value_ptr(proj));
    }

    void draw() override {
        glUseProgram(shader);
        glUniform3fv(glGetUniformLocation(shader, "uLightColor"),1 ,glm::value_ptr((lightColor *brightness)));
        mesh.draw();
    }

    glm::mat4 getModelTransform() override {
        return modelTransform;
    }

    glm::mat4 modelTransform;
    glm::vec3 lightColor = glm::vec3(1, 1, 1);
    float brightness = 1;
    GLuint shader;
    cgra::gl_mesh mesh;

};

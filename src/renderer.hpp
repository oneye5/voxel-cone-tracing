#include <renderable.hpp>
#include <vector>
#include <vct/gBufferPrepass.hpp>
#include <vct/gBufferLightingPass.hpp>
#include <vct/voxelizer.hpp>

class Renderer {
public:
    gBufferPrepass* prepass;
    gBufferLightingPass* lightingPass; 
    Voxelizer* voxelizer;
    std::vector<Renderable*> renderables;

    Renderer(int width, int height) {
        prepass = new gBufferPrepass(width, height);
        lightingPass = new gBufferLightingPass(prepass);
        voxelizer = new Voxelizer(512);
        currentProj = glm::mat4(1);
        currentView = glm::mat4(1);
    }

    void addRenderable(Renderable* r) {
        renderables.push_back(r);
    }

    void resizeWindow(int w, int h) {
        prepass->resize(w, h);
    }

    void render(glm::mat4& view, glm::mat4& proj) {
        auto shaders = getShaders();
        auto modelMatricies = getModelMatricies();
        currentProj = proj;
        currentView = view;

        voxelizer->voxelize([&]() { drawAllWithoutSetUniforms(); }, modelMatricies, shaders);
        prepass->executePrepass(shaders, [&]() {drawAll(); });
        lightingPass->runPass();
    }

    glm::mat4 currentView;
    glm::mat4 currentProj;

    std::vector<glm::mat4> getModelMatricies() {
        std::vector<glm::mat4> out{};
        for (auto obj : renderables) {
            out.push_back(obj->getModelTransform());
        }
        return out;
    }
    std::vector<GLuint> getShaders() {
        std::vector<GLuint> out{};
        for (auto obj : renderables) {
            out.push_back(obj->getShader());
        }
        return out;
    }
    void drawAllWithoutSetUniforms() {
        for (auto obj : renderables) {
            obj->draw();
        }
    }
    void drawAll() {
        for (auto obj : renderables) {
            obj->setProjViewUniforms(currentView, currentProj);
            obj->draw();
        }
    }
};

#include <renderable.hpp>
#include <vector>
#include <vct/gBufferPrepass.hpp>
#include <vct/gBufferLightingPass.hpp>
#include <vct/voxelizer.hpp>
#ifndef BAKINGBAD_RENDERER_H
#define BAKINGBAD_RENDERER_H

struct debug_parameters {
    bool voxel_debug_mode_on;
    float voxel_slice;
    int debug_channel_index; // shared channel index
    bool gbuffer_debug_mode_on;
};

class Renderer {
public:
    gBufferPrepass* prepass;
    gBufferLightingPass* lightingPass; 
    Voxelizer* voxelizer;
    std::vector<Renderable*> renderables;
    debug_parameters debug_params;

    Renderer(int width, int height) {
        prepass = new gBufferPrepass(width, height);
        voxelizer = new Voxelizer(512);
        lightingPass = new gBufferLightingPass(prepass, voxelizer);
        currentProj = glm::mat4(1);
        currentView = glm::mat4(1);
    
        // set params
        debug_params.debug_channel_index = 0;
        debug_params.gbuffer_debug_mode_on = false;
        debug_params.voxel_debug_mode_on = false;
        debug_params.voxel_slice = 0;
    }

    void addRenderable(Renderable* r) {
        renderables.push_back(r);
    }

    void resizeWindow(int w, int h) {
        prepass->resize(w, h);
    }

    // call if the scene changes
    void refreshVoxels(glm::mat4& view, glm::mat4& proj) {
        auto shaders = getShaders();
        auto modelMatricies = getModelMatricies();
        voxelizer->voxelize([&]() { drawAllWithoutSetUniforms(); }, modelMatricies, shaders);
    }

    void render(glm::mat4& view, glm::mat4& proj) {
        glDisable(GL_CULL_FACE);
        cleanDebugParams();
        auto shaders = getShaders();
        auto modelMatricies = getModelMatricies();
        currentProj = proj;
        currentView = view;

  
        if (debug_params.voxel_debug_mode_on) {
            voxelizer->renderDebugSlice(debug_params.voxel_slice, debug_params.debug_channel_index);
            return;
        }

        prepass->executePrepass(shaders, [&]() {drawAll(); });

        lightingPass->runPass(view ,debug_params.gbuffer_debug_mode_on ? debug_params.debug_channel_index : 0);
    }

    void cleanDebugParams() { // make sure the params make sense, eg. only one debug mode is on
        if (debug_params.gbuffer_debug_mode_on && debug_params.voxel_debug_mode_on)
            debug_params.gbuffer_debug_mode_on = false;
    }

    glm::mat4 currentView;
    glm::mat4 currentProj;

    std::vector<glm::mat4> getModelMatricies() {
        std::vector<glm::mat4> out{};
        for (auto obj : renderables) {
            auto shaders = obj->getShaders();
            for(auto x : shaders)
                out.push_back(obj->getModelTransform());
        }
        return out;
    }
    std::vector<GLuint> getShaders() {
        std::vector<GLuint> out{};
        for (auto obj : renderables) {
            auto shaders = obj->getShaders();
            for (auto s : shaders) 
                out.push_back(s);
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
#endif

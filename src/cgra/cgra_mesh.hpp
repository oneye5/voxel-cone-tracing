
#pragma once

// std
#include <iostream>
#include <vector>

// glm
#include <glm/glm.hpp>

// project
#include <opengl.hpp>



namespace cgra {

	// A data structure for holding buffer IDs and other information related to drawing.
	// Also has a helper functions for drawing the mesh and deleting the gl buffers.
	// location 1 : positions (vec3)
	// location 2 : normals (vec3)
	// location 3 : uv (vec2)
	struct gl_mesh {
		GLuint vao = 0;
		GLuint vbo = 0;
		GLuint ibo = 0;
		GLenum mode = 0; // mode to draw in, eg: GL_TRIANGLES
		int index_count = 0; // how many indicies to draw (no primitives)

		// calls the draw function on mesh data
		void draw();

		// deletes the gl buffers (cleans up all the data)
		void destroy();
	};


	struct mesh_vertex {
		glm::vec3 pos{0};
		glm::vec3 norm{0};
		glm::vec2 uv{0};
	};


	// Mesh builder object used to create an mesh by taking vertex and index information
	// and uploading them to OpenGL.
	struct mesh_builder {

		GLenum mode = GL_TRIANGLES;
		std::vector<mesh_vertex> vertices;
		std::vector<unsigned int> indices;

		mesh_builder() {}

		mesh_builder(GLenum mode_) : mode(mode_) {}

		template <size_t N, size_t M>
		explicit mesh_builder(const mesh_vertex(&vertData)[N], const mesh_vertex(&idxData)[M], GLenum mode_ = GL_TRIANGLES)
			: vertices(vertData, vertData+N), indices(idxData, idxData+M), mode(mode_) { }

		GLuint push_vertex(mesh_vertex v) {
			auto size = vertices.size();
			assert(size == decltype(size)(GLuint(size)));
			vertices.push_back(v);
			return GLuint(size);
		}

		void push_index(GLuint i) {
			indices.push_back(i);
		}

		void push_indices(std::initializer_list<GLuint> inds) {
			indices.insert(indices.end(), inds);
		}

		gl_mesh build() const;

		void print() const {
			std::cout << "pos" << std::endl;
			for (mesh_vertex v : vertices) {
				std::cout << v.pos.x << ", " << v.pos.y << ", " << v.pos.z << ", ";
				std::cout << v.norm.x << ", " << v.norm.y << ", " << v.norm.z << ", ";
				std::cout << v.uv.x << ", " << v.uv.y << ", " << std::endl;
			}
			std::cout << "idx" << std::endl;
			for (int i : indices) {
				std::cout << i << ", ";
			}
			std::cout << std::endl;
		}
	};

	// Create and return a mesh builder containing a plane from 0,0,0 to 1,0,1 subdivided by the amount specified
	static mesh_builder CREATE_PLANE(int x_sub, int z_sub) {
		mesh_builder mb;

		const float x_step = 1.0f / static_cast<float>(x_sub);
		const float z_step = 1.0f / static_cast<float>(z_sub);

		for (float x = 0.0; x <= 1.0; x += x_step) {
			for (float z = 0.0; z <= 1.0; z += z_step) {
				mb.push_vertex({{x, 0.0, z}, {0.0, 1.0, 0.0}, {x, z}});
			}
		}

		for (int z = 0; z < z_sub; z++) {
			for (int x = 0; x < x_sub; x++) {
				GLuint tl = z * (x_sub + 1) + x; // top left
				GLuint tr = tl + 1;
				GLuint bl = tl + (x_sub + 1);
				GLuint br = bl + 1;

				mb.push_indices({tl, tr, bl});
				mb.push_indices({bl, tr, br});
			}
		}

		return mb;
	}
}


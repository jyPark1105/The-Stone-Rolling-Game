#pragma once


#include <vector>
#include "shader.h"
#include "loader.h"
#include <exception>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <tuple>
#include "camera.h"
#include "model.h"
#include <random>


class Lane
{
	Shader shader;
	unsigned int VAO;
	unsigned int VBO;
	unsigned int EBO;
	unsigned int texture;
	unsigned int elnum;
	std::vector<glm::vec2> treePos;

public:
	Lane() = delete;

	Lane(std::initializer_list<std::pair<float, float>> lanes, const char* texture_path, const char* vertex_shader, const char* fragment_shader, float width = 4.0):
		texture(loadTexture(texture_path)),
		shader(vertex_shader, fragment_shader)
	{
		if (lanes.size() < 1) {
			throw std::runtime_error("Size of lanes should greater than 0");
		}

		elnum = (unsigned int) lanes.size() * 6;
		std::random_device rd;
		std::mt19937 gen(rd());
		std::normal_distribution<float> dist(0.f, width / 4.f);

		// prevLeft, prevRight, nextLeft, nextRight
		std::vector<float> vertices;
		std::vector<unsigned int> indices;
		vertices.reserve((lanes.size() + 1) << 2);
		indices.reserve(elnum);
		treePos.reserve(lanes.size());

		unsigned int offset = 0;
		glm::vec2 prevPoint(0.f);
		glm::vec2 Dir(0.f, -1.f), sDir;
		glm::vec2 nextLeft, nextRight;
		glm::vec2 orthoDir; //left

		width = width / 2;

		auto iter = lanes.begin();

		// First pass
		float angle, length;
		std::tie(angle, length) = *(iter++);

		Dir = glm::mat2(glm::cos(angle), glm::sin(angle), -glm::sin(angle), glm::cos(angle)) * Dir;
		orthoDir = glm::vec2(-Dir.y, Dir.x) * width;
		nextLeft = orthoDir;
		nextRight = -orthoDir;

		push_vec2(vertices, nextLeft);
		push_vec2(vertices, nextRight);

		sDir = Dir * length;
		nextLeft = sDir + orthoDir;
		nextRight = sDir - orthoDir;


		treePos.push_back(prevPoint + sDir * 0.5f + orthoDir * 2.f + glm::vec2(dist(gen), dist(gen)));
		treePos.push_back(prevPoint + sDir * 0.5f - orthoDir * 2.f + glm::vec2(dist(gen), dist(gen)));
		prevPoint += sDir;

		push_vec2(vertices, nextLeft);
		push_vec2(vertices, nextRight);

		push_indices(indices, offset);

		offset += 2;

		// Left ones
		for (; iter != lanes.end(); iter++, offset += 2)
		{
			std::tie(angle, length) = *iter;

			Dir = glm::mat2(glm::cos(angle), glm::sin(angle), -glm::sin(angle), glm::cos(angle)) * Dir;
			orthoDir = glm::vec2(-Dir.y, Dir.x) * width;
			sDir = Dir * length;
			nextLeft = prevPoint + sDir + orthoDir;
			nextRight = prevPoint + sDir - orthoDir;

			treePos.push_back(prevPoint + sDir * 0.5f + orthoDir * 4.f + glm::vec2(dist(gen), dist(gen)));
			treePos.push_back(prevPoint + sDir * 0.5f - orthoDir * 4.f+ glm::vec2(dist(gen), dist(gen)));
			prevPoint += sDir;

			push_vec2(vertices, nextLeft);
			push_vec2(vertices, nextRight);

			push_indices(indices, offset);
		}

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);
		// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		shader.use();
		shader.setInt("lane", 0);
	}

	~Lane()
	{
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
		glDeleteTextures(1, &texture);
	}

	void draw(Camera& camera, glm::mat4& projection, Model* tree = nullptr, Shader* treeShader = nullptr)
	{
		shader.use();

		glm::mat4 mvp = projection * camera.GetViewMatrix();
		shader.setMat4("mvp", mvp);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, elnum, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		if (tree)
		{
			treeShader->use();
			glm::mat4 mvp_tree;
			for (glm::vec2& pos : treePos) 
			{
				mvp_tree = mvp * glm::translate(glm::mat4(1.f), glm::vec3(pos.x, 0.f, pos.y));
				treeShader->setMat4("mvp", mvp_tree);
				tree->Draw(*treeShader);
			}
		}
	}

private:
	void push_vec2(std::vector<float>& vertices, glm::vec2 vec)
	{
		vertices.insert(vertices.end(), {vec.x, vec.y});
	}

	void push_indices(std::vector<unsigned int>& indices, unsigned int offset)
	{
		indices.insert(indices.end(),
			{offset, offset+1, offset+2,
			 offset+1, offset+2, offset+3});
	}
};
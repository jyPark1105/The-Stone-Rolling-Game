#pragma once


#include <vector>
#include "headers/shader.h"
#include "loader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "headers/camera.h"
#include "model.h"
#include "wall.h"


class Rectangle
{
	unsigned int VAO;
	unsigned int VBO;
	unsigned int EBO;
	unsigned int instanceBuffer;
	unsigned int diffuseTexture;
	unsigned int normalTexture;
	unsigned int roughnessTexture;
	unsigned int aoTexture;
	unsigned int elnum;
	// std::vector<glm::mat4> instanceMatrices;
	Wall wall;

public:
	glm::mat4 model;

	Rectangle() = delete;

	// Assume the direction is -z
	Rectangle(glm::vec3 start, float width, float length, const char* texture_path, const char* normal_path, const char* roughness_path, const char* ao_path, float radian, const char* wall_texture_path) :
		diffuseTexture(loadTexture(texture_path)),
		normalTexture(loadTexture(normal_path)),
		roughnessTexture(loadTexture(roughness_path)),
		aoTexture(loadTexture(ao_path))
	{
		// Coords
		glm::vec3 tl, tr, bl, br;
		br = start + glm::vec3(width / 2, 0.f, 0.f);
		bl = start + glm::vec3(-width / 2, 0.f, 0.f);
		tr = start + glm::vec3(width / 2, 0.f, -length);
		tl = start + glm::vec3(-width / 2, 0.f, -length);

		float vertices[] = {
			bl.x, bl.y, bl.z,
			br.x, br.y, br.z,
			tl.x, tl.y, tl.z,
			tr.x, tr.y, tr.z
		};

		model = glm::translate(glm::mat4(1.f), start);
		model = glm::rotate(model, radian, glm::vec3(1.f, 0.f, 0.f));
		model = glm::translate(model, -start);
		

		unsigned int indices[] = { 0,1,2,1,2,3 };

		elnum = sizeof(indices) / sizeof(indices[0]);

		wall = Wall(wall_texture_path,
        					glm::vec3(0.f, 0.f, -1.f),
       						glm::vec3(1.f, 0.f, 0.f),
        					0.3f, 2.f, length,
							{ glm::vec4(br + glm::vec3(0.f, 0.f, -length/2.f), glm::radians(0.f)),
							  glm::vec4(bl + glm::vec3(0.f, 0.f, -length/2.f), glm::radians(180.f)) });
		

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		// // Instance buffer
        // glGenBuffers(1, &instanceBuffer);
        // glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
        // glBufferData(GL_ARRAY_BUFFER, instanceMatrices.size() * sizeof(glm::mat4), &instanceMatrices[0], GL_STATIC_DRAW);

        // glBindVertexArray(VAO);
        // // set attribute pointers for matrix (4 times vec4)
        // glEnableVertexAttribArray(2);
        // glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
        // glEnableVertexAttribArray(3);
        // glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
        // glEnableVertexAttribArray(4);
        // glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
        // glEnableVertexAttribArray(5);
        // glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

        // glVertexAttribDivisor(2, 1);
        // glVertexAttribDivisor(3, 1);
        // glVertexAttribDivisor(4, 1);
        // glVertexAttribDivisor(5, 1);

        // glBindVertexArray(0);

	}

	~Rectangle()
	{
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
		glDeleteTextures(1, &diffuseTexture);
		glDeleteTextures(1, &normalTexture);
		glDeleteTextures(1, &roughnessTexture);
		glDeleteTextures(1, &aoTexture);
	}

	void draw(Shader& rectShader, Shader& wallShader)//, glm::mat4& view, glm::mat4& projection, Shader& wallShader)
	{
		rectShader.use();

		// glm::mat4 vp = projection * view;
		// rectShader.setMat4("vp", vp);
		rectShader.setMat4("model", model);
		rectShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normalTexture);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, roughnessTexture);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, aoTexture);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, elnum, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		wall.draw(wallShader, model);
	}

	static void setShader(Shader& rectShader)
	{
		rectShader.use();
		rectShader.setInt("texture_diffuse1", 0);
		rectShader.setInt("texture_normal1", 1);
		rectShader.setInt("texture_roughness1", 2);
		rectShader.setInt("texture_ao1", 3);
	}
};
#pragma once


#include <vector>
#include "headers/shader.h"
#include "loader.h"
#include <exception>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "model.h"
#include <limits.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class Column
{
	Model model;
    int num_columns;
    float radius;
    float radian;
    glm::mat4* modelMatrices;

public:
	glm::vec3 aabb1_min = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	glm::vec3 aabb1_max = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	Column() = delete;

	// Assume the direction is z
	Column(const char* model_path, int num_columns, float radian, float radius) : model(model_path), num_columns(num_columns), radian(radian), radius(radius)
	{
        modelMatrices = new glm::mat4[num_columns];
        float angle = (float)(M_PI - radian) / 2;
        float delta_angle = radian / (num_columns-1);
        for (int i = 0; i < num_columns; i++)
        {
            modelMatrices[i] = glm::translate(glm::mat4(1.f), glm::vec3(radius * -cos(angle), 1.f, radius * sin(angle)));
            modelMatrices[i] = glm::rotate(modelMatrices[i], angle, glm::vec3(0.f, 1.f, 0.f));
            
            angle += delta_angle;
        }

        model.setInstanced(num_columns, modelMatrices);

		Mesh& mesh = model.meshes[0];
		for (unsigned int v = 0; v < mesh.vertices.size(); ++v) {
			Vertex& vertex = mesh.vertices[v];

			aabb1_min = glm::min(aabb1_min, vertex.Position);

			aabb1_max = glm::max(aabb1_max, vertex.Position);
		}
	}

	~Column()
	{
        delete[] modelMatrices;
	}

	void draw(Shader& columnShader)//, glm::mat4& view, glm::mat4& projection)
	{
		columnShader.use();

		// columnShader.setMat4("vp", projection * view);
		model.DrawInstanced(columnShader, num_columns);
	}
};
#pragma once

#include <vector>
#include "headers/shader.h"
#include "headers/camera.h"
#include "loader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class Circle
{
private:
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;
    unsigned int vertexCount;
    glm::vec3 center;
    float radius;
    int triangleCount;
    unsigned int texture;

public:
    Circle() = delete;

    // Constructor: position of center, number of triangles, radius
    Circle(glm::vec3 position, int numTriangles, float circleRadius, const char* texture_path)
        : center(position), radius(circleRadius), triangleCount(numTriangles), texture(loadTexture(texture_path))
    {
        generateCircleGeometry();
    }

    ~Circle()
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }

    void draw(Shader& shader)//, glm::mat4& view, glm::mat4& projection)
    {
        shader.use();
        
        // Create model matrix for positioning the circle
        glm::mat4 model = glm::translate(glm::mat4(1.0f), center);
        // glm::mat4 vp = projection * view;
        // shader.setMat4("vp", vp);
        shader.setMat4("model", model);
        
        glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    static void setShader(Shader& shader)
	{
		shader.use();
		shader.setInt("circle", 0);
	}

private:
    void generateCircleGeometry()
    {
        std::vector<float> vertices;
        std::vector<unsigned int> indices;
        vertices.reserve((triangleCount + 2)*3);
        indices.reserve(triangleCount * 3);

        // Center vertex (in local coordinates, will be translated by model matrix)
        vertices.push_back(0.0f); // x
        vertices.push_back(0.0f); // y
        vertices.push_back(0.0f); // z

        // Generate vertices around the circle in x-z plane
        for (int i = 0; i <= triangleCount; ++i)
        {
            float angle = (float)(2.0f * M_PI * i / triangleCount);
            float x = radius * cos(angle);
            float z = radius * sin(angle);
            
            vertices.push_back(x);   // x
            vertices.push_back(0.0f); // y (circle lies in x-z plane)
            vertices.push_back(z);   // z
        }

        // Generate indices for triangles
        for (int i = 0; i < triangleCount; ++i)
        {
            indices.push_back(0);           // center vertex
            indices.push_back(i + 1);       // current edge vertex
            indices.push_back(i + 2);       // next edge vertex
        }

        vertexCount = (unsigned int) indices.size();

        // Generate and bind VAO
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        // Bind and fill VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        // Bind and fill EBO
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        // Set vertex attribute pointers
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Unbind VAO
        glBindVertexArray(0);
    }
};

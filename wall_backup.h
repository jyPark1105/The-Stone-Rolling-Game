#pragma once

#include <glm/glm.hpp>
#include <glad/glad.h>
#include "shader.h"
#include "loader.h"


class Wall
{
    unsigned int VAO, VBO, EBO;
    unsigned int num_indices;

public:
    static unsigned int texture;
    Wall() = delete;

    // Assume the direction is -z
    Wall(const char* texture_path, glm::vec3 position, glm::vec3 dir, glm::vec3 ortho, float width, float height, float length)
    {
        if(texture == 0 && texture_path)
        {
            Wall::texture = loadTexture(texture_path);
        }
        glm::vec3 br = position;
        glm::vec3 bl = br + dir * length;
        glm::vec3 tl = bl + glm::vec3(0.f, height, 0.f);
        glm::vec3 tr = br + glm::vec3(0.f, height, 0.f);

        glm::vec3 br_w = br + ortho * width;
        glm::vec3 bl_w = bl + ortho * width;
        glm::vec3 tl_w = tl + ortho * width;
        glm::vec3 tr_w = tr + ortho * width;

        float vertices[] = {
            br.x, br.y, br.z, 0.0f, 0.0f,
            bl.x, bl.y, bl.z, length, 0.0f,
            tl.x, tl.y, tl.z, length, height,
            tr.x, tr.y, tr.z, 0.0f, height,
            
            br_w.x, br_w.y, br_w.z, width, 0.0f,
            bl_w.x, bl_w.y, bl_w.z, length + width, 0.0f,
            tl_w.x, tl_w.y, tl_w.z, length + width, height,
            tr_w.x, tr_w.y, tr_w.z, width, height,

            tl_w.x, tl_w.y, tl_w.z, length, height + width,
            tr_w.x, tr_w.y, tr_w.z, 0.0f, height + width
        };

        unsigned int indices[] = {
            0, 1, 2,
            0, 2, 3,
            0, 3, 4,
            3, 4, 7,
            2, 5, 6,
            2, 5, 1,
            2, 3, 9,
            2, 8, 9
        };

        num_indices = sizeof(indices) / sizeof(indices[0]);

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    ~Wall()
    {
    }

    void draw(Shader& shader, glm::mat4& model)//, glm::mat4& vp, glm::mat4& model)
    {
        shader.use();
        // shader.setMat4("vp", vp);
        shader.setMat4("model", model);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Wall::texture);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    static void setShader(Shader& shader)
    {
        shader.use();
        shader.setInt("wall", 0);
    }
};
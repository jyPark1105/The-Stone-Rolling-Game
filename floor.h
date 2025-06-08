#pragma once

#include "headers/shader.h"
#include "headers/camera.h"
#include "loader.h"
#include <GLFW/glfw3.h>
#include <tuple>

class Floor
{
    Shader shader;
    unsigned int VAO;
    unsigned int VBO;
    unsigned int texture;
    float floorsize;

public:
    Floor(const char *texturePath, const char *vertexPath, const char *fragmentPath, float floorsize = 100.f):
        shader(vertexPath, fragmentPath),
        texture(loadTexture(texturePath)),
        floorsize(floorsize)
    {
        std::tie(VAO, VBO) = initFloorArrays();

        shader.use();
        shader.setInt("flr", 0);
    }

    Floor() = delete;

    ~Floor()
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteTextures(1, &texture);
    }

    void draw(Camera& camera, glm::mat4& projection)
    {
        shader.use();

        glm::vec3 cpos = camera.Position;
        glm::mat4 model = glm::translate(glm::mat4(1.f), glm::vec3(cpos.x, 0.f, cpos.z));
        glm::mat4 mvp = projection * camera.GetViewMatrix() * model;
        shader.setMat4("mvp", mvp);
        shader.setMat4("model", model);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

private:
    // Returns a pair of VAO and VBO for the floor
    std::pair<unsigned int, unsigned int> initFloorArrays()
    {
        float vertices[] = {
            -floorsize, -floorsize,
            floorsize, -floorsize,
            floorsize, floorsize,
            -floorsize, -floorsize,
            floorsize, floorsize,
            -floorsize, floorsize};

        unsigned int VBO, VAO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        return std::make_pair(VAO, VBO);
    }
};
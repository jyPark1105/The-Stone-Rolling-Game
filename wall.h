#pragma once

#include <glm/glm.hpp>
#include <glad/glad.h>
#include "headers/shader.h"
#include "loader.h"


class Wall
{
    unsigned int VAO, VBO, EBO, instanceBuffer;
    unsigned int num_indices;
    std::vector<glm::mat4> instanceMatrices;

public:
    static unsigned int texture;
    static unsigned int normal;
    static unsigned int roughness;
    static unsigned int ao;
    //Wall() = delete;
    Wall():
        VAO(0),
        VBO(0),
        EBO(0),
        instanceBuffer(0),
        num_indices(0)
    {}


    // Assume the direction is -z
    Wall(const char* texture_path, glm::vec3 dir, glm::vec3 ortho, float width, float height, float length, std::initializer_list<glm::vec4> positions)
    {
        if(texture == 0 && texture_path)
        {
            Wall::texture = loadTexture(texture_path);
        }

        glm::vec3 bitan = glm::cross(dir, ortho);

        glm::vec3 origin = glm::vec3(0.f, 0.f, 0.f);
        glm::vec3 br = origin - dir * length / 2.f;
        glm::vec3 bl = origin + dir * length / 2.f;
        glm::vec3 tl = bl + glm::vec3(0.f, height, 0.f);
        glm::vec3 tr = br + glm::vec3(0.f, height, 0.f);

        glm::vec3 br_w = br + ortho * width;
        glm::vec3 bl_w = bl + ortho * width;
        glm::vec3 tl_w = tl + ortho * width;
        glm::vec3 tr_w = tr + ortho * width;

        struct wallVertex {
            // position
            glm::vec3 Position;
            // normal
            glm::vec3 Normal;
            // texCoords
            glm::vec2 TexCoords;
            // tangent
            glm::vec3 Tangent;
            // bitangent
            glm::vec3 Bitangent;
        }vertices[16];

        // front
        vertices[0].Position = br;
        vertices[0].Normal = -ortho;
        vertices[0].TexCoords = glm::vec2(0.f, 0.f);
        vertices[0].Tangent = dir;
        vertices[0].Bitangent = bitan;

        vertices[1].Position = bl;
        vertices[1].Normal = -ortho;
        vertices[1].TexCoords = glm::vec2(length, 0.f);
        vertices[1].Tangent = dir;
        vertices[1].Bitangent = bitan;

        vertices[2].Position = tl;
        vertices[2].Normal = -ortho;
        vertices[2].TexCoords = glm::vec2(length, height);
        vertices[2].Tangent = dir;
        vertices[2].Bitangent = bitan;

        vertices[3].Position = tr;
        vertices[3].Normal = -ortho;
        vertices[3].TexCoords = glm::vec2(0.f, height);
        vertices[3].Tangent = dir;
        vertices[3].Bitangent = bitan;

        // right
        vertices[4].Position = br_w;
        vertices[4].Normal = -dir;
        vertices[4].TexCoords = glm::vec2(width, 0.0f);
        vertices[4].Tangent = ortho;
        vertices[4].Bitangent = -bitan;

        vertices[5].Position = br;
        vertices[5].Normal = -dir;
        vertices[5].TexCoords = glm::vec2(0.f, 0.f);
        vertices[5].Tangent = ortho;
        vertices[5].Bitangent = -bitan;

        vertices[6].Position = tr;
        vertices[6].Normal = -dir;
        vertices[6].TexCoords = glm::vec2(0.f, height);
        vertices[6].Tangent = ortho;
        vertices[6].Bitangent = -bitan;

        vertices[7].Position = tr_w;
        vertices[7].Normal = -dir;
        vertices[7].TexCoords = glm::vec2(width, height);
        vertices[7].Tangent = ortho;
        vertices[7].Bitangent = -bitan;

        // left
        vertices[8].Position = bl;
        vertices[8].Normal = dir;
        vertices[8].TexCoords = glm::vec2(length, 0.f);
        vertices[8].Tangent = ortho;
        vertices[8].Bitangent = bitan;

        vertices[9].Position = bl_w;
        vertices[9].Normal = dir;
        vertices[9].TexCoords = glm::vec2(length + width, 0.0f);
        vertices[9].Tangent = ortho;
        vertices[9].Bitangent = bitan;

        vertices[10].Position = tl_w;
        vertices[10].Normal = dir;
        vertices[10].TexCoords = glm::vec2(length + width, height);
        vertices[10].Tangent = ortho;
        vertices[10].Bitangent = bitan;

        vertices[11].Position = tl;
        vertices[11].Normal = dir;
        vertices[11].TexCoords = glm::vec2(length, height);
        vertices[11].Tangent = ortho;
        vertices[11].Bitangent = bitan;

        // top
        vertices[12].Position = tr_w;
        vertices[12].Normal = -bitan;
        vertices[12].TexCoords = glm::vec2(0.0f, height + width);
        vertices[12].Tangent = ortho;
        vertices[12].Bitangent = dir;

        vertices[13].Position = tr;
        vertices[13].Normal = -bitan;
        vertices[13].TexCoords = glm::vec2(0.f, height);
        vertices[13].Tangent = ortho;
        vertices[13].Bitangent = dir;

        vertices[14].Position = tl;
        vertices[14].Normal = -bitan;
        vertices[14].TexCoords = glm::vec2(length, height);
        vertices[14].Tangent = ortho;
        vertices[14].Bitangent = dir;

        vertices[15].Position = tl_w;
        vertices[15].Normal = -bitan;
        vertices[15].TexCoords = glm::vec2(length, height + width);
        vertices[15].Tangent = ortho;
        vertices[15].Bitangent = dir;

        unsigned int indices[] = {
            0, 1, 2,
            0, 2, 3,

            4, 5, 6,
            4, 6, 7,

            8, 9, 10,
            8, 10, 11,

            12, 13, 14,
            12, 14, 15
        };


        // float vertices[] = {
        //     // Position, TexCoord
        //     br.x, br.y, br.z, 0.0f, 0.0f,
        //     bl.x, bl.y, bl.z, length, 0.0f,
        //     tl.x, tl.y, tl.z, length, height,
        //     tr.x, tr.y, tr.z, 0.0f, height,
            
        //     br_w.x, br_w.y, br_w.z, width, 0.0f,
        //     bl_w.x, bl_w.y, bl_w.z, length + width, 0.0f,
        //     tl_w.x, tl_w.y, tl_w.z, length + width, height,
        //     tr_w.x, tr_w.y, tr_w.z, width, height,

        //     tl_w.x, tl_w.y, tl_w.z, length, height + width,
        //     tr_w.x, tr_w.y, tr_w.z, 0.0f, height + width
        // };

        // unsigned int indices[] = {
        //     0, 1, 2,
        //     0, 2, 3,

        //     0, 3, 4,
        //     3, 4, 7,

        //     2, 5, 6,
        //     2, 5, 1,

        //     2, 3, 9,
        //     2, 8, 9
        // };

        for(auto& pos : positions)
        {
            glm::mat4 model = glm::mat4(1.f);
            model = glm::translate(model, glm::vec3(pos.x, pos.y, pos.z));
            model = glm::rotate(model, pos.w, glm::vec3(0.f, 1.f, 0.f));
            instanceMatrices.push_back(model);
        }

        num_indices = sizeof(indices) / sizeof(indices[0]);

        // glGenVertexArrays(1, &VAO);
        // glGenBuffers(1, &VBO);
        // glGenBuffers(1, &EBO);

        // glBindVertexArray(VAO);
        // glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        // glEnableVertexAttribArray(0);

        // glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        // glEnableVertexAttribArray(1);

        // glBindBuffer(GL_ARRAY_BUFFER, 0);
        // glBindVertexArray(0);

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

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(wallVertex), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(wallVertex), (void*)offsetof(Vertex, Normal));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(wallVertex), (void*)offsetof(Vertex, TexCoords));
        glEnableVertexAttribArray(2);

        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(wallVertex), (void*)offsetof(Vertex, Tangent));
        glEnableVertexAttribArray(3);

        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(wallVertex), (void*)offsetof(Vertex, Bitangent));
        glEnableVertexAttribArray(4);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        // Instance buffer
        glGenBuffers(1, &instanceBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
        glBufferData(GL_ARRAY_BUFFER, instanceMatrices.size() * sizeof(glm::mat4), &instanceMatrices[0], GL_STATIC_DRAW);

        glBindVertexArray(VAO);
        // set attribute pointers for matrix (4 times vec4)
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
        glEnableVertexAttribArray(7);
        glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
        glEnableVertexAttribArray(8);
        glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);
        glVertexAttribDivisor(7, 1);
        glVertexAttribDivisor(8, 1);

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
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, Wall::normal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, Wall::roughness);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, Wall::ao);
        glBindVertexArray(VAO);
        glDrawElementsInstanced(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, 0, (unsigned int)instanceMatrices.size());
        glBindVertexArray(0);
    }

    static void loadTextures(const char* texture_path, const char* normal_path, const char* roughness_path, const char* ao_path)
    {
        if(texture == 0 && texture_path)
        {
            Wall::texture = loadTexture(texture_path);
        }
        if(normal == 0 && normal_path)
        {
            Wall::normal = loadTexture(normal_path);
        }
        if(roughness == 0 && roughness_path)
        {
            Wall::roughness = loadTexture(roughness_path);
        }
        if(ao == 0 && ao_path)
        {
            Wall::ao = loadTexture(ao_path);
        }
    }

    static void setShader(Shader& shader)
    {
        shader.use();
        shader.setInt("texture_diffuse1", 0);
        shader.setInt("texture_normal1", 1);
        shader.setInt("texture_roughness1", 2);
        shader.setInt("texture_ao1", 3);
    }
};
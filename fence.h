#pragma once

#include "model.h"
#include <GLFW/glfw3.h>


class Fence {
    Model model;
    std::vector<glm::mat4> modelMatrices;
    std::vector<unsigned int> instanceBuffers;
    std::vector<int> instanceBools;
    public:
        glm::vec3 aabb1_min = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	    glm::vec3 aabb1_max = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

        Fence(const char* modelPath, glm::mat4& _model, std::initializer_list<glm::vec4> pos):
            model(modelPath)
        {
            Mesh& mesh = model.meshes[0];
            for (unsigned int v = 0; v < mesh.vertices.size(); ++v) {
                Vertex& vertex = mesh.vertices[v];

                aabb1_min = glm::min(aabb1_min, vertex.Position);

                aabb1_max = glm::max(aabb1_max, vertex.Position);
            }

            const float ratio = 1.4f;
            float height = (aabb1_max.y - aabb1_min.y) * 0.1f * ratio;

            glm::mat4 modelMatrixDefault;

            modelMatrixDefault = glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(0.0f, 0.f, -1.f));
            modelMatrixDefault = glm::scale(modelMatrixDefault, glm::vec3(0.1f, 0.2f, 0.1f) * ratio);

            modelMatrices.reserve(pos.size());
            instanceBools.reserve(pos.size());

            for (auto& p : pos) {
                modelMatrices.push_back(_model * glm::translate(glm::mat4(1.f), glm::vec3(p.x, p.y + height / 2.f, p.z)) * modelMatrixDefault);
                instanceBools.push_back((int)(p.w));
            }

            model.setInstanced((unsigned int)modelMatrices.size(), modelMatrices.data());

            instanceBuffers.resize(model.meshes.size());

            for(unsigned int i = 0; i < model.meshes.size(); i++){
                unsigned int VAO = model.meshes[i].VAO;
                glBindVertexArray(VAO);

                glGenBuffers(1, &instanceBuffers[i]);
                glBindBuffer(GL_ARRAY_BUFFER, instanceBuffers[i]);
                glBufferData(GL_ARRAY_BUFFER, (unsigned int)instanceBools.size() * sizeof(int), &instanceBools[0], GL_STATIC_DRAW);

                glEnableVertexAttribArray(11);
                glVertexAttribIPointer(11, 1, GL_INT, sizeof(int), (void*)0);
                
                glVertexAttribDivisor(11, 1);

                glBindVertexArray(0);
            }
        }

        void draw(Shader& shader)//, glm::mat4& view, glm::mat4& projection)
        {
            shader.use();
            // shader.setMat4("vp", projection * view);

            // Rotate the fence
            float angle = static_cast<float>(glfwGetTime()) * 40.f;
            shader.setMat4("model", glm::rotate(glm::mat4(1.f), glm::radians(angle), glm::vec3(1.0f, 0.f, 0.f)));

            // Draw fence
            model.DrawInstanced(shader, (unsigned int)modelMatrices.size());
        }
};

#pragma once

#include "model.h"
#include <GLFW/glfw3.h>


class Obstacle {
    Model model;
    std::vector<glm::mat4> modelMatrices;
    public:
        glm::vec3 aabb1_min = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	    glm::vec3 aabb1_max = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
        glm::vec3 objSize;

        Obstacle(const char* modelPath, glm::mat4& _model, std::vector<glm::vec3>& pos):
            model(modelPath)
        {
            Mesh& mesh = model.meshes[0];
            for (unsigned int v = 0; v < mesh.vertices.size(); ++v) {
                Vertex& vertex = mesh.vertices[v];

                aabb1_min = glm::min(aabb1_min, vertex.Position);

                aabb1_max = glm::max(aabb1_max, vertex.Position);
            }
            
            // glm::mat4 M = glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(0.0f, 0.f, -1.f));
            // glm::vec4 tmp = M * glm::vec4(aabb1_min, 1.f);
            // aabb1_min = glm::vec3(tmp.x, tmp.y, tmp.z);
            // tmp = M * glm::vec4(aabb1_max, 1.f);
            // aabb1_max = glm::vec3(tmp.x, tmp.y, tmp.z);

            glm::vec3 tmp = aabb1_min;
            aabb1_min = glm::vec3(aabb1_min.y, -aabb1_max.x, aabb1_min.z);
            aabb1_max = glm::vec3(aabb1_max.y, -tmp.x, aabb1_max.z);

            const float ratio = 0.05f;

            // 물체 크기
            objSize = ratio * (aabb1_max - aabb1_min);

            // 보정용(원점)
            glm::mat4 defaultModel = glm::mat4(1.f);
            defaultModel = glm::translate(defaultModel, -(aabb1_min + aabb1_max) * ratio / 2.f);
            defaultModel = glm::rotate(defaultModel, glm::radians(90.f), glm::vec3(-1.f, 0.f, 0.f));
            defaultModel = glm::scale(defaultModel, glm::vec3(ratio));

            for (auto& p : pos) {
                modelMatrices.push_back(_model * glm::translate(glm::mat4(1.f), p) * defaultModel);
            }

            model.setInstanced((unsigned int)modelMatrices.size(), modelMatrices.data());
        }

        void draw(Shader& shader)//, glm::mat4& view, glm::mat4& projection)
        {
            shader.use();

            // Draw obstacle
            model.DrawInstanced(shader, (unsigned int)modelMatrices.size());
        }

        std::vector<glm::vec3> getWorldPositions() const {
            std::vector<glm::vec3> result;
            for (const auto& mat : modelMatrices) {
                glm::vec4 worldPos = mat * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); // 모델 중심
                result.push_back(glm::vec3(worldPos));
            }
            return result;
        }
};

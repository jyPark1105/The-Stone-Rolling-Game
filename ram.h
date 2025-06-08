#pragma once

class Ram {
    Model log, workbench;
    std::vector<glm::mat4> logMatrices, workbenchMatrices;

public:
    glm::vec3 aabb1_min = glm::vec3(FLT_MAX);
    glm::vec3 aabb1_max = glm::vec3(-FLT_MAX);
    glm::vec3 objSize;

    Ram(const char* logPath, const char* workbenchPath, std::vector<glm::vec3> pos)
        : log(logPath), workbench(workbenchPath)
    {
        Mesh& mesh = log.meshes[0];
        for (auto& vertex : mesh.vertices) {
            aabb1_min = glm::min(aabb1_min, vertex.Position);
            aabb1_max = glm::max(aabb1_max, vertex.Position);
        }

        const float logScale = 0.014f;
        const float workbenchScale = 0.01f;

        objSize = logScale * (aabb1_max - aabb1_min);

        for (auto& p : pos) {
            logMatrices.push_back(glm::translate(glm::mat4(1.f), p) * glm::scale(glm::mat4(1.f), glm::vec3(logScale)));
            workbenchMatrices.push_back(glm::translate(glm::mat4(1.f), p) * glm::scale(glm::mat4(1.f), glm::vec3(workbenchScale)));
        }

        log.setInstanced((unsigned int)logMatrices.size(), logMatrices.data());
        workbench.setInstanced((unsigned int)workbenchMatrices.size(), workbenchMatrices.data());
    }

    std::vector<glm::vec3> getWorldPositions() const {
        std::vector<glm::vec3> result;
        for (const auto& mat : logMatrices) {
            glm::vec4 worldPos = mat * glm::vec4(0, 0, 0, 1);
            result.push_back(glm::vec3(worldPos));
        }
        return result;
    }

    void draw(Shader& shader, glm::mat4& _model)
    {
        shader.use();
        float t = static_cast<float>(glfwGetTime()) * 2.f;

        shader.setMat4("model", _model * glm::translate(glm::mat4(1.f), glm::vec3(0.f, 1.8f, glm::max(glm::sin(t) * 3.f, 0.f))));
        log.DrawInstanced(shader, (unsigned int)logMatrices.size());

        shader.setMat4("model", _model);
        workbench.DrawInstanced(shader, (unsigned int)workbenchMatrices.size());
    }
};
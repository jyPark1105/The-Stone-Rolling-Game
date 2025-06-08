#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "shader.h"

class ParticleSystem {
public:
    struct Particle {
        glm::vec3 position;
        glm::vec3 velocity;
        float life;
        float size;
        bool active;
    };

    ParticleSystem(unsigned int maxCount = 200, Shader* externalShader = nullptr)
        : maxParticles(maxCount), shader(externalShader)
    {
        particles.resize(maxParticles);
        initMesh();
    }

    void initMesh() 
    {
        float quad[] = {
            -0.5f, -0.5f, 0.0f,
             0.5f, -0.5f, 0.0f,
            -0.5f,  0.5f, 0.0f,
             0.5f,  0.5f, 0.0f
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glBindVertexArray(0);
    }

    void update(float dt) 
    {
        for (auto& p : particles) 
        {
            if (!p.active) continue;
            p.life -= dt;
            if (p.life <= 0.0f) 
            {
                p.active = false;
                continue;
            }
            p.position += p.velocity * dt;
        }
    }

    void spawn(const glm::vec3& pos) 
    {
        int spawned = 0;
        for (auto& p : particles) 
        {
            if (!p.active) 
            {
                p.position = pos + glm::vec3(0.0f, 0.05f, 0.0f);
                p.velocity = glm::vec3(
                    (rand() % 100 - 50) / 100.0f,
                    (rand() % 100) / 100.0f + 1.5f,
                    (rand() % 100 - 50) / 100.0f
                );

                p.life = 0.5f + (rand() % 50) / 100.0f; // 0.5~1.0s
                p.size = 0.05f + (rand() % 10) / 100.0f; // 0.05~0.15
                p.active = true;
                if (++spawned >= 30) break;  // 최대 30개 생성
            }
        }
    }

    void render(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraRight, const glm::vec3& cameraUp) 
    {
        shader->use();
        shader->setMat4("V", view);
        shader->setMat4("P", projection);
        glBindVertexArray(VAO);
        for (auto& p : particles) 
        {
            if (!p.active) continue;
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, p.position);
            model[0] = glm::vec4(cameraRight * p.size, 0.0f);
            model[1] = glm::vec4(cameraUp * p.size, 0.0f);
            shader->setMat4("M", model);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
        glBindVertexArray(0);
    }

private:
    std::vector<Particle> particles;
    unsigned int maxParticles;
    unsigned int VAO, VBO;
    Shader* shader;
};

#endif

#ifndef OPENGL_DEBUG_DRAWER_H
#define OPENGL_DEBUG_DRAWER_H

#include <bullet/btBulletDynamicsCommon.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>
#include "shader.h"

extern Shader* debugShader;

class OpenGLDebugDrawer : public btIDebugDraw {
public:
    int debugMode;
    std::vector<float> lineVertices;
    GLuint vao = 0, vbo = 0;
    bool initialized = false;

    glm::mat4 viewMatrix = glm::mat4(1.0f);
    glm::mat4 projectionMatrix = glm::mat4(1.0f);

    OpenGLDebugDrawer() {
        debugMode = DBG_DrawWireframe;
    }

    ~OpenGLDebugDrawer() {
        if (vao) glDeleteVertexArrays(1, &vao);
        if (vbo) glDeleteBuffers(1, &vbo);
    }

    void setViewProjection(const glm::mat4& view, const glm::mat4& proj) {
        viewMatrix = view;
        projectionMatrix = proj;
    }

    void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override {
        lineVertices.insert(lineVertices.end(), {
            from.getX(), from.getY(), from.getZ(), color.getX(), color.getY(), color.getZ(),
            to.getX(),   to.getY(),   to.getZ(),   color.getX(), color.getY(), color.getZ()
            });
    }

    void flushLines() override {
        if (lineVertices.empty()) {
            return;
        }

        if (!initialized) {
            glGenVertexArrays(1, &vao);
            glGenBuffers(1, &vbo);
            initialized = true;
        }

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, lineVertices.size() * sizeof(float), lineVertices.data(), GL_DYNAMIC_DRAW);

        // Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

        // Color
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

        debugShader->use();
        debugShader->setMat4("V", viewMatrix);
        debugShader->setMat4("P", projectionMatrix);

        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(lineVertices.size() / 6));

        glBindVertexArray(0);
        lineVertices.clear();
    }

    void setDebugMode(int mode) override {
        debugMode = mode;
    }

    int getDebugMode() const override {
        return debugMode;
    }

    void drawContactPoint(
        const btVector3& PointOnB,
        const btVector3& normalOnB,
        btScalar distance,
        int lifeTime,
        const btVector3& color
    ) override {
    }

    void reportErrorWarning(const char* warningString) override {
        std::cout << "[Bullet Warning] " << warningString << std::endl;
    }

    void draw3dText(const btVector3&, const char*) override {
    }
};

#endif

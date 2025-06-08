#pragma once

#include "model.h"
#include "headers/shader.h"
#include "loader.h"

class Ball {

    unsigned int diffuseMap;
    unsigned int normalMap;
    unsigned int roughnessMap;
    unsigned int aoMap;
    unsigned int displacement;
    void (*renderFunc)();
    
public:
    Ball(const char* diffuseMapPath, const char* normalMapPath, const char* roughnessMapPath, const char* aoMapPath, const char* displacementPath, void (*func)()):
        diffuseMap(loadTexture(diffuseMapPath)),
        normalMap(loadTexture(normalMapPath)),
        roughnessMap(loadTexture(roughnessMapPath)),
        aoMap(loadTexture(aoMapPath)),
        displacement(loadTexture(displacementPath)),
        renderFunc(func)
    {
    }
    
    void Draw(Shader& shader)
    {
        shader.use();
        shader.setInt("texture_diffuse1", 0);
        shader.setInt("texture_normal1", 1);
        shader.setInt("texture_roughness1", 2);
        shader.setInt("texture_ao1", 3);
        shader.setInt("texture_displacement1", 4);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normalMap);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, roughnessMap);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, aoMap);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, displacement);
        renderFunc();
    }
private:
};

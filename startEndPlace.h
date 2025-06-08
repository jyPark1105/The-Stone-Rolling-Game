#pragma once

#include "rectangle.h"
#include "wall.h"

class StartEndPlace
{
    Rectangle rectangle;
    Wall wall1;
    Wall wall2;

    StartEndPlace(glm::vec3 start, float width, float length, const char* texture_path, float radian, const char* wall_texture_path) :
        rectangle(start, width, length, texture_path, radian, wall_texture_path)
    {
        wall1 = Wall(wall_texture_path, glm::vec3(width/2.f, 0.f, -1.f), glm::vec3(1.f, 0.f, 0.f), 0.3f, 2.f, length, { glm::vec4(rectangle.model * glm::vec4(rectangle.br, 0.f, 0.f)) });
        wall2 = Wall(wall_texture_path, glm::vec3(0.f, 0.f, -1.f), glm::vec3(1.f, 0.f, 0.f), 0.3f, 2.f, length, { glm::vec4(rectangle.model * glm::vec4(rectangle.bl, 0.f, 0.f)) });
    }

    void draw(Shader& rectShader, Shader& wallShader)
    {
        rectangle.draw(rectShader, wallShader);
        wall1.draw(wallShader, rectangle.model);
        wall2.draw(wallShader, rectangle.model);
    }
};

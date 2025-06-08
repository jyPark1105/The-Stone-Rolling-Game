#ifndef BONETRANSFORMCACHE_H
#define BONETRANSFORMCACHE_H

#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>
#include <string>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

class BoneTransformCache {
private:
    std::unordered_map<unsigned int, std::vector<GLint>> cache;

public:
    void build(unsigned int shaderID) {
        if (cache.find(shaderID) != cache.end()) return;

        std::vector<GLint> locations;
        for (int i = 0; i < 100; ++i) {
            std::string name = "boneTransforms[" + std::to_string(i) + "]";
            GLint loc = glGetUniformLocation(shaderID, name.c_str());
            locations.push_back(loc);
        }
        cache[shaderID] = locations;
    }

    void setBoneTransforms(unsigned int shaderID, const std::vector<glm::mat4>& transforms) {
        const std::vector<GLint>& locs = cache[shaderID];
        for (size_t i = 0; i < transforms.size(); ++i) {
            if (locs[i] != -1)
                glUniformMatrix4fv(locs[i], 1, GL_FALSE, glm::value_ptr(transforms[i]));
        }
    }
};

#endif

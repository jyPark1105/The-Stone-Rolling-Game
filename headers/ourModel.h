#ifndef OURMODEL_H
#define OURMODEL_H

#include <functional>

enum TextureType {
    OBJ_DIFFUSE,
    OBJ_NORMAL,
    OBJ_SPECULAR,
    OBJ_AO,
};

struct TextureOverride {
    int meshIndex;
    TextureType type;
    std::string path;
};

class OurModel {
public:
    std::vector<ObjectMesh> meshes;
    std::vector<unsigned int> vaos;
    std::vector<unsigned int> indexCounts;

    unsigned int diffuseMap = 0;
    unsigned int normalMap = 0;
    unsigned int specularMap = 0;
    unsigned int aoMap = 0;

    std::string modelPath; // 헤더에 추가

    OurModel(const std::string& objPath,
        const std::string& diffuseTex,
        const std::string& normalTex,
        const std::string& specularTex) 
    {
        this->modelPath = objPath;

        loadModel(objPath);
        diffuseMap = textureFromFile(diffuseTex);
        normalMap = textureFromFile(normalTex);
        specularMap = textureFromFile(specularTex);

        // VAO 캐싱
        for (auto& mesh : meshes) {
            unsigned int vao = generateBuffer(mesh);
            vaos.push_back(vao);
            indexCounts.push_back(mesh.indices.size());
        }
    }

    OurModel(const std::string& objPath, const std::vector<TextureOverride>& overrides)
    {
        this->modelPath = objPath;

        loadModel(objPath);

        for (const auto & override : overrides) {
            unsigned int texID = textureFromFile(override.path);
            switch (override.type) {
            case OBJ_DIFFUSE:       diffuseMap = texID; break;
            case OBJ_NORMAL:        normalMap = texID; break;
            case OBJ_SPECULAR:      specularMap = texID; break;
            case OBJ_AO:            aoMap = texID; break;
            }
        }

        for (auto& mesh : meshes) {
            unsigned int vao = generateBuffer(mesh);
            vaos.push_back(vao);
            indexCounts.push_back(mesh.indices.size());
        }
    }

    // Y Bot
    void Draw(Shader& shader) {
        shader.use();

        shader.setInt("texture_diffuse1", 0);
        shader.setInt("texture_normal1", 1);
        shader.setInt("texture_specular1", 2);

        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, diffuseMap);
        glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, normalMap);
        glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, specularMap);

        for (size_t i = 0; i < vaos.size(); ++i) {
            glBindVertexArray(vaos[i]);
            glDrawElements(GL_TRIANGLES, indexCounts[i], GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }
        //std::cout << "여기 실행됨. Draw()" << std::endl;
    }
    
    // Stone Ball
    void Draw(Shader& shader, bool isTexOverride) {
        if (isTexOverride) {
            shader.use();

            shader.setInt("texture_diffuse1", 0);
            shader.setInt("texture_normal1", 1);
            shader.setInt("texture_specular1", 2);
            shader.setInt("texture_ao1", 3);

            glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, diffuseMap);
            glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, normalMap);
            glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, specularMap);
            glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, aoMap);

            for (size_t i = 0; i < vaos.size(); ++i) {
                glBindVertexArray(vaos[i]);
                glDrawElements(GL_TRIANGLES, indexCounts[i], GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);
            }
        }
    }


    glm::vec3 getAABBSize() const {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(this->modelPath,
            aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);

        if (!scene || !scene->mRootNode) {
            std::cerr << "[OurModel::getAABBSize] Failed to load scene for AABB: " << this->modelPath << std::endl;
            return glm::vec3(1.0f); // fallback
        }

        glm::vec3 minBounds(FLT_MAX);
        glm::vec3 maxBounds(-FLT_MAX);

        std::function<void(aiNode*, const aiMatrix4x4&)> computeBounds =
            [&](aiNode* node, const aiMatrix4x4& parentTransform) {
            aiMatrix4x4 globalTransform = parentTransform * node->mTransformation;

            for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
                aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
                for (unsigned int j = 0; j < mesh->mNumVertices; ++j) {
                    aiVector3D v = globalTransform * mesh->mVertices[j];
                    minBounds.x = std::min(minBounds.x, v.x);
                    minBounds.y = std::min(minBounds.y, v.y);
                    minBounds.z = std::min(minBounds.z, v.z);

                    maxBounds.x = std::max(maxBounds.x, v.x);
                    maxBounds.y = std::max(maxBounds.y, v.y);
                    maxBounds.z = std::max(maxBounds.z, v.z);
                }
            }

            for (unsigned int i = 0; i < node->mNumChildren; ++i)
                computeBounds(node->mChildren[i], globalTransform);
            };

        computeBounds(scene->mRootNode, aiMatrix4x4());

        return maxBounds - minBounds; // = AABB size
    }


private:
    void loadModel(const std::string& path) {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
            return;
        }
        processNode(scene->mRootNode, scene);
    }

    void processNode(aiNode* node, const aiScene* scene) {
        for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh));
        }

        for (unsigned int i = 0; i < node->mNumChildren; ++i) {
            processNode(node->mChildren[i], scene);
        }
    }

    ObjectMesh processMesh(aiMesh* mesh) {
        ObjectMesh result;

        for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
            result.vertices.push_back(glm::vec3(
                mesh->mVertices[i].x,
                mesh->mVertices[i].y,
                mesh->mVertices[i].z
            ));
            
            if (mesh->HasNormals()) {
                result.normals.push_back(glm::vec3(
                    mesh->mNormals[i].x,
                    mesh->mNormals[i].y,
                    mesh->mNormals[i].z
                ));
            }

            if (mesh->mTextureCoords[0])
                result.textureCoordinates.push_back(glm::vec2(
                    mesh->mTextureCoords[0][i].x,
                    mesh->mTextureCoords[0][i].y
                ));
            else
                result.textureCoordinates.push_back(glm::vec2(0.0f, 0.0f));

            // Tangent & Bitangent도 채워야 한다면 여기서 처리
            if (mesh->HasTangentsAndBitangents()) {
                result.tangents.push_back(glm::vec3(
                    mesh->mTangents[i].x,
                    mesh->mTangents[i].y,
                    mesh->mTangents[i].z
                ));
                result.bitangents.push_back(glm::vec3(
                    mesh->mBitangents[i].x,
                    mesh->mBitangents[i].y,
                    mesh->mBitangents[i].z
                ));
            }
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
            for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; ++j)
                result.indices.push_back(mesh->mFaces[i].mIndices[j]);
        }

        return result;
    }


    unsigned int textureFromFile(const std::string& path) {
        unsigned int textureID;
        glGenTextures(1, &textureID);

        int width, height, nrComponents;
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
        if (data) {
            GLenum format = (nrComponents == 1) ? GL_RED :
                (nrComponents == 3) ? GL_RGB :
                (nrComponents == 4) ? GL_RGBA : GL_RGB;

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
        }
        else {
            std::cerr << "Failed to load texture at path: " << path << std::endl;
            stbi_image_free(data);
        }

        return textureID;
    }
};

#endif
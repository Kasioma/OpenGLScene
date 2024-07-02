#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/transform.hpp>

#include "Model3D.hpp"

class Object {
   public:
    void LoadModel(std::string fileName) { m_model.LoadModel(fileName); }

    void LoadModel(std::string fileName, std::string basePath) { m_model.LoadModel(fileName, basePath); }

    void Draw(gps::Shader shader, bool setNormalMatrix = true) {
        shader.useShaderProgram();

        shader.setUniform("model", m_modelMatix);

        if (setNormalMatrix) {
            shader.setUniform("normalMatrix", m_normalMatrix);
        }

        m_model.Draw(shader);
    }

    void ResetModelMatrix() { m_modelMatix = glm::mat4(1.0f); }

    void Translate(glm::vec3 translate) { m_modelMatix = glm::translate(m_modelMatix, translate); };

    void Scale(glm::vec3 scale) { m_modelMatix = glm::scale(m_modelMatix, scale); };

    void Rotate(float angle, glm::vec3 axis) {
        axis         = glm::normalize(axis);
        m_modelMatix = glm::rotate(m_modelMatix, glm::radians(angle), axis);
    };

    void ComputeNormalMatrix(glm::mat4 viewMatrix) {
        m_normalMatrix = glm::mat3(glm::inverseTranspose(viewMatrix * m_modelMatix));
    }

   private:
    gps::Model3D m_model;

    glm::mat4 m_modelMatix   = glm::mat4(1.0f);
    glm::mat3 m_normalMatrix = glm::mat3(1.0f);
};

#endif  // OBJECT_HPP

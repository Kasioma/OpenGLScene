//
//  Shader.cpp
//  Lab3
//
//  Created by CGIS on 05/10/2016.
//  Copyright © 2016 CGIS. All rights reserved.
//

#include "Shader.hpp"

#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>

namespace gps {
    std::string Shader::readShaderFile(std::string fileName) {
        std::ifstream shaderFile;
        std::string shaderString;

        // open shader file
        shaderFile.open(fileName);

        std::stringstream shaderStringStream;

        // read shader content into stream
        shaderStringStream << shaderFile.rdbuf();

        // close shader file
        shaderFile.close();

        // convert stream into GLchar array
        shaderString = shaderStringStream.str();
        return shaderString;
    }

    void Shader::loadShader(std::string vertexShaderFileName, std::string fragmentShaderFileName) {
        // read, parse and compile the vertex shader
        std::string v                    = readShaderFile(vertexShaderFileName);
        const GLchar* vertexShaderString = v.c_str();
        GLuint vertexShader;
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderString, NULL);
        glCompileShader(vertexShader);

        // read, parse and compile the vertex shader
        std::string f                      = readShaderFile(fragmentShaderFileName);
        const GLchar* fragmentShaderString = f.c_str();
        GLuint fragmentShader;
        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderString, NULL);
        glCompileShader(fragmentShader);

        // attach and link the shader programs
        this->shaderProgram = glCreateProgram();
        glAttachShader(this->shaderProgram, vertexShader);
        glAttachShader(this->shaderProgram, fragmentShader);
        glLinkProgram(this->shaderProgram);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    void Shader::useShaderProgram() {
        glUseProgram(this->shaderProgram);
    }

    int Shader::getUniform(const std::string& uniformName) {
        if (m_uniformLocCache.find(uniformName) != m_uniformLocCache.end())
            return m_uniformLocCache[uniformName];

        int loc                        = glGetUniformLocation(shaderProgram, uniformName.c_str());
        m_uniformLocCache[uniformName] = loc;

        if (loc == -1)
            printf("Unknown uniform '%s'\n", uniformName.c_str());

        return loc;
    }

    void Shader::setUniform(const std::string& uniformName, int value) {
        int loc = getUniform(uniformName);
        glUniform1i(loc, value);
    }
    void Shader::setUniform(const std::string& uniformName, float value) {
        int loc = getUniform(uniformName);
        glUniform1f(loc, value);
    }
    void Shader::setUniform(const std::string& uniformName, glm::vec2 vec) {
        int loc = getUniform(uniformName);
        glUniform2f(loc, vec.x, vec.y);
    }
    void Shader::setUniform(const std::string& uniformName, glm::vec3 vec) {
        int loc = getUniform(uniformName);
        glUniform3f(loc, vec.x, vec.y, vec.z);
    }

    void Shader::setUniform(const std::string& uniformName, glm::vec4 vec) {
        int loc = getUniform(uniformName);
        glUniform4f(loc, vec.x, vec.y, vec.z, vec.w);
    }

    void Shader::setUniform(const std::string& uniformName, glm::mat3 mat) {
        int loc = getUniform(uniformName);
        glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(mat));
    }

    void Shader::setUniform(const std::string& uniformName, glm::mat4 mat) {
        int loc = getUniform(uniformName);
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mat));
    }

}  // namespace gps

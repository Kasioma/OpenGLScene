//
//  Shader.hpp
//  Lab3
//
//  Created by CGIS on 05/10/2016.
//  Copyright © 2016 CGIS. All rights reserved.
//

#ifndef Shader_hpp
#define Shader_hpp

#include <unordered_map>
#if defined(__APPLE__)
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#include <glm/glm.hpp>
#include <string>

namespace gps {

    class Shader {
       public:
        GLuint shaderProgram;
        void loadShader(std::string vertexShaderFileName, std::string fragmentShaderFileName);
        void useShaderProgram();

        void setUniform(const std::string& uniformName, int value);
        void setUniform(const std::string& uniformName, float value);
        void setUniform(const std::string& uniformName, glm::vec2 vec);
        void setUniform(const std::string& uniformName, glm::vec3 vec);
        void setUniform(const std::string& uniformName, glm::vec4 vec);
        void setUniform(const std::string& uniformName, glm::mat3 mat);
        void setUniform(const std::string& uniformName, glm::mat4 mat);

       private:
        std::unordered_map<std::string, int> m_uniformLocCache;
        int getUniform(const std::string& uniformName);

        std::string readShaderFile(std::string fileName);
    };

}  // namespace gps

#endif /* Shader_hpp */

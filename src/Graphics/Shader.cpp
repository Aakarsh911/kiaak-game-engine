#include "Graphics/Shader.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <GLFW/glfw3.h>

namespace Kiaak {

Shader::Shader() : programID(0), isCompiled(false) {
    // Initialize the shader program ID
    programID = glCreateProgram();
    if (programID == 0) {
        std::cerr << "Failed to create shader program" << std::endl;
    }
}

Shader::~Shader() {
    if (programID != 0) {
        glDeleteProgram(programID);
    }
}

bool Shader::LoadFromFile(const std::string& vertexPath, const std::string& fragmentPath) {
    // Load shader sources from files
    std::string vertexCode;
    std::string fragmentCode;

    // Read vertex shader file
    std::ifstream vertexFile(vertexPath);
    if (vertexFile) {
        vertexCode.assign((std::istreambuf_iterator<char>(vertexFile)), std::istreambuf_iterator<char>());
        vertexFile.close();
    } else {
        std::cerr << "Failed to open vertex shader file: " << vertexPath << std::endl;
        return false;
    }

    // Read fragment shader file
    std::ifstream fragmentFile(fragmentPath);
    if (fragmentFile) {
        fragmentCode.assign((std::istreambuf_iterator<char>(fragmentFile)), std::istreambuf_iterator<char>());
        fragmentFile.close();
    } else {
        std::cerr << "Failed to open fragment shader file: " << fragmentPath << std::endl;
        return false;
    }

    unsigned int vertexShader, fragmentShader;
    if (!CompileShader(vertexCode, GL_VERTEX_SHADER, vertexShader) ||
        !CompileShader(fragmentCode, GL_FRAGMENT_SHADER, fragmentShader)) {
        return false;
    }

    // Attach shaders to the program
    glAttachShader(programID, vertexShader);
    glAttachShader(programID, fragmentShader);

    // Link the program
    glLinkProgram(programID);

    // Check for linking errors
    if (!CheckProgramErrors(programID)) {
        return false;
    }

    // Clean up shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    isCompiled = true;
    return true;
}

void Shader::Use() {
    if (isCompiled) {
        glUseProgram(programID);
    }
}

bool Shader::CompileShader(const std::string& source, unsigned int type, unsigned int& shaderId) {
    shaderId = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shaderId, 1, &src, nullptr);
    glCompileShader(shaderId);

    return CheckShaderErrors(shaderId, type == GL_VERTEX_SHADER ? "vertex" : "fragment");
}

bool Shader::CheckShaderErrors(unsigned int shader, const std::string& type) {
    int success;
    char infoLog[512];
    
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "Shader compilation error of type: " << type << "\n" << infoLog << std::endl;
        return false;
    }
    return true;
}

bool Shader::CheckProgramErrors(unsigned int program) {
    int success;
    char infoLog[512];
    
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "Shader program linking error: " << infoLog << std::endl;
        return false;
    }
    return true;
}

// Uniform setters
void Shader::SetBool(const std::string& name, bool value) {
    glUniform1i(glGetUniformLocation(programID, name.c_str()), (int)value);
}

void Shader::SetInt(const std::string& name, int value) {
    glUniform1i(glGetUniformLocation(programID, name.c_str()), value);
}

void Shader::SetFloat(const std::string& name, float value) {
    glUniform1f(glGetUniformLocation(programID, name.c_str()), value);
}

void Shader::SetVec2(const std::string& name, const glm::vec2& value) {
    glUniform2fv(glGetUniformLocation(programID, name.c_str()), 1, &value[0]);
}

void Shader::SetVec3(const std::string& name, const glm::vec3& value) {
    glUniform3fv(glGetUniformLocation(programID, name.c_str()), 1, &value[0]);
}

void Shader::SetVec4(const std::string& name, const glm::vec4& value) {
    glUniform4fv(glGetUniformLocation(programID, name.c_str()), 1, &value[0]);
}

void Shader::SetMat3(const std::string& name, const glm::mat3& value) {
    glUniformMatrix3fv(glGetUniformLocation(programID, name.c_str()), 1, GL_FALSE, &value[0][0]);
}

void Shader::SetMat4(const std::string& name, const glm::mat4& value) {
    glUniformMatrix4fv(glGetUniformLocation(programID, name.c_str()), 1, GL_FALSE, &value[0][0]);
}

} // namespace Kiaak
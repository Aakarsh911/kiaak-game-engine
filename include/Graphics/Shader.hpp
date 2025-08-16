#pragma once

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Kiaak {

class Shader {
public:
    Shader();
    ~Shader();

    bool LoadFromFile(const std::string& vertexPath, const std::string& fragmentPath);
    void Use();

    // Uniform setters
    void SetBool(const std::string& name, bool value);
    void SetInt(const std::string& name, int value);
    void SetFloat(const std::string& name, float value);
    void SetVec2(const std::string& name, const glm::vec2& value);
    void SetVec3(const std::string& name, const glm::vec3& value);
    void SetVec4(const std::string& name, const glm::vec4& value);
    void SetMat3(const std::string& name, const glm::mat3& value);
    void SetMat4(const std::string& name, const glm::mat4& value);

private:
    bool CompileShader(const std::string& source, unsigned int type, unsigned int& shaderId);
    bool CheckShaderErrors(unsigned int shader, const std::string& type);
    bool CheckProgramErrors(unsigned int program);

    unsigned int programID;
    bool isCompiled;
};

} // namespace Kiaak
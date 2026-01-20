#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

namespace netra::graphics {

class Shader {
public:
    Shader() = default;
    Shader(const std::string& vertex_src, const std::string& fragment_src);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    void use() const;
    GLuint id() const { return m_id; }

    void set_int(const std::string& name, int value) const;
    void set_float(const std::string& name, float value) const;
    void set_vec2(const std::string& name, const glm::vec2& value) const;
    void set_vec3(const std::string& name, const glm::vec3& value) const;
    void set_vec4(const std::string& name, const glm::vec4& value) const;
    void set_mat4(const std::string& name, const glm::mat4& value) const;

private:
    GLuint m_id = 0;
    GLuint compile_shader(GLenum type, const std::string& source);
};

} // namespace netra::graphics

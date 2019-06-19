#include "shader_program.h"

#include "panic.h"

#include <array>
#include <fstream>
#include <sstream>

#include <glm/gtc/type_ptr.hpp>

shader_program::shader_program()
    : id_{glCreateProgram()}
{
}

void shader_program::add_shader(GLenum type, std::string_view filename)
{
    const auto shader_id = glCreateShader(type);

    const auto source = [filename] {
        std::ifstream file{filename.data()};
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }();
    const auto source_ptr = source.data();
    glShaderSource(shader_id, 1, &source_ptr, nullptr);
    glCompileShader(shader_id);

    int status;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &status);
    if (!status)
    {
        std::array<GLchar, 64 * 1024> buf;
        GLsizei length;
        glGetShaderInfoLog(shader_id, buf.size() - 1, &length, buf.data());
        panic("failed to compile shader %s:\n%.*s", std::string(filename).c_str(), length, buf.data());
    }

    glAttachShader(id_, shader_id);
}

void shader_program::link()
{
    glLinkProgram(id_);

    int status;
    glGetProgramiv(id_, GL_LINK_STATUS, &status);
    if (!status)
        panic("failed to link shader program\n");
}

void shader_program::bind()
{
    glUseProgram(id_);
}

int shader_program::uniform_location(std::string_view name) const
{
    return glGetUniformLocation(id_, name.data());
}

void shader_program::set_uniform(int location, float value)
{
    glUniform1f(location, value);
}

void shader_program::set_uniform(int location, const glm::vec2 &value)
{
    glUniform2fv(location, 1, glm::value_ptr(value));
}

void shader_program::set_uniform(int location, const glm::vec3 &value)
{
    glUniform3fv(location, 1, glm::value_ptr(value));
}

void shader_program::set_uniform(int location, const glm::vec4 &value)
{
    glUniform4fv(location, 1, glm::value_ptr(value));
}

void shader_program::set_uniform(int location, const glm::mat3 &value)
{
    glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

void shader_program::set_uniform(int location, const glm::mat4 &value)
{
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

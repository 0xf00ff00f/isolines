#pragma once

#include "noncopyable.h"

#include <GL/glew.h>

#include <array>
#include <string_view>

#include <glm/glm.hpp>

class shader_program : private noncopyable
{
public:
    shader_program();

    void add_shader(GLenum type, std::string_view path);
    void link();

    void bind();

    int uniform_location(std::string_view name) const;

    void set_uniform(int location, float v);
    void set_uniform(int location, const glm::vec2 &v);
    void set_uniform(int location, const glm::vec3 &v);
    void set_uniform(int location, const glm::vec4 &v);
    void set_uniform(int location, const glm::mat3 &mat);
    void set_uniform(int location, const glm::mat4 &mat);

private:
    GLuint id_;
};

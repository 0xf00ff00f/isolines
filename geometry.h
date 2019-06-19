#pragma once

#include "noncopyable.h"

#include <vector>

#include <GL/glew.h>

class geometry : private noncopyable
{
public:
    geometry()
    {
        glGenBuffers(1, &vbo_);
        glGenVertexArrays(1, &vao_);
    }

    ~geometry()
    {
        glDeleteBuffers(1, &vbo_);
        glDeleteVertexArrays(1, &vao_);
    }

    struct vertex_attrib
    {
        GLint size;
        GLenum type;
        std::size_t offset;
    };

    template<typename T>
    void set_data(const std::vector<T> &buf, const std::vector<vertex_attrib> &attribs)
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(T) * buf.size(), buf.data(), GL_STATIC_DRAW);

        glBindVertexArray(vao_);
        for (int i = 0; i < attribs.size(); ++i)
        {
            const auto &attrib = attribs[i];
            glEnableVertexAttribArray(i);
            glVertexAttribPointer(i, attrib.size, attrib.type, GL_FALSE, sizeof(T),
                                  reinterpret_cast<GLvoid *>(attrib.offset));
        }
    }

    void bind() { glBindVertexArray(vao_); }

private:
    GLuint vao_;
    GLuint vbo_;
};

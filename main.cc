#include "panic.h"

#include "geometry.h"
#include "shader_program.h"
#include "noise.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <functional>
#include <iostream>

// #define DUMP_FRAMES

constexpr auto window_width = 512;
constexpr auto window_height = 512;

int main(int argc, char *argv[])
{
    if (!glfwInit())
        panic("glfwInit failed\n");

    glfwSetErrorCallback([](int error, const char *description) { panic("GLFW error: %s\n", description); });

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_SAMPLES, 16);
    auto *window = glfwCreateWindow(window_width, window_height, "demo", nullptr, nullptr);
    if (!window)
        panic("glfwCreateWindow failed\n");

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glewInit();

    glfwSetKeyCallback(window, [](GLFWwindow *window, int key, int scancode, int action, int mode) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);
    });

    const auto projection =
        glm::perspective(glm::radians(45.0f), static_cast<float>(window_width) / window_height, 0.1f, 100.f);
    const auto view = glm::lookAt(glm::vec3(0, 0, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

    struct vertex
    {
        glm::vec3 position;
        float noise;
    };
    std::vector<vertex> verts;
    constexpr auto max_subdivision = 7;
    std::function<void(const glm::vec3 &, const glm::vec3 &, const glm::vec3 &, int)> subdivide_triangle =
        [&verts, &subdivide_triangle](const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, int level) {
            if (level == max_subdivision)
            {
                const auto noise = [](const glm::vec3 &v) {
                    const float scale = 5.0f;
                    return 0.5f + 0.5f * ::noise(scale * v.x, scale * v.y, scale * v.z);
                };
                const auto v0n = glm::normalize(v0);
                const auto v1n = glm::normalize(v1);
                const auto v2n = glm::normalize(v2);
                verts.push_back({v0n, noise(v0n)});
                verts.push_back({v1n, noise(v1n)});
                verts.push_back({v2n, noise(v2n)});
            }
            else
            {
                const auto v01 = 0.5f * (v0 + v1);
                const auto v12 = 0.5f * (v1 + v2);
                const auto v20 = 0.5f * (v2 + v0);
                subdivide_triangle(v0, v01, v20, level + 1);
                subdivide_triangle(v01, v1, v12, level + 1);
                subdivide_triangle(v20, v12, v2, level + 1);
                subdivide_triangle(v01, v12, v20, level + 1);
            }
        };

    const auto v1 = glm::vec3{0.000000, 0.000000, 0.612372};
    const auto v2 = glm::vec3{-0.288675, -0.500000, -0.204124};
    const auto v3 = glm::vec3{-0.288675, 0.500000, -0.204124};
    const auto v4 = glm::vec3{0.577350, 0.000000, -0.204124};
    subdivide_triangle(v2, v3, v4, 0);
    subdivide_triangle(v3, v2, v1, 0);
    subdivide_triangle(v4, v1, v2, 0);
    subdivide_triangle(v1, v4, v3, 0);

    std::cout << verts.size() << " verts, " << (verts.size() / 3) << " tris\n";

    geometry triangle_geometry;
    triangle_geometry.set_data(verts,
                               {{3, GL_FLOAT, offsetof(vertex, position)}, {1, GL_FLOAT, offsetof(vertex, noise)}});

    shader_program program;
    program.add_shader(GL_VERTEX_SHADER, "shaders/isolines.vert");
    program.add_shader(GL_GEOMETRY_SHADER, "shaders/isolines.geom");
    program.add_shader(GL_FRAGMENT_SHADER, "shaders/isolines.frag");
    program.link();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_MULTISAMPLE);

#ifdef DUMP_FRAMES
    constexpr auto total_frames = 160;
    auto frame_num = 0;
    std::vector<char> frame_data;
    frame_data.resize(window_width * window_height * 4);
#else
    float cur_time = 0.0;
#endif

    while (!glfwWindowShouldClose(window))
    {
        glViewport(0, 0, window_width, window_height);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#ifdef DUMP_FRAMES
        float angle = 0.25f * sinf(static_cast<float>(frame_num) * 2.0f * M_PI / total_frames);
#else
        float angle = 0.25f * sinf(cur_time);
#endif
        const auto model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0));

        const auto mvp = projection * view * model;

        glm::mat3 modelNormal(model);
        modelNormal = glm::inverse(modelNormal);
        modelNormal = glm::transpose(modelNormal);

        program.bind();
        program.set_uniform(program.uniform_location("mvp"), mvp);
        program.set_uniform(program.uniform_location("normalMatrix"), modelNormal);
        program.set_uniform(program.uniform_location("lightPosition"), glm::vec3(7, 7, 5));

        constexpr auto num_levels = 17;
        constexpr float step = 1.0f / (num_levels + 1);
        for (int level = 0; level < num_levels; ++level)
        {
            const auto threshold = (level + 1) * step;
            program.set_uniform(program.uniform_location("threshold"), threshold);

            triangle_geometry.bind();
            glDrawArrays(GL_TRIANGLES, 0, verts.size());
        }

#ifdef DUMP_FRAMES
        glReadPixels(0, 0, window_width, window_height, GL_RGBA, GL_UNSIGNED_BYTE, frame_data.data());

        char path[80];
        std::sprintf(path, "%05d.ppm", frame_num);

        if (auto *out = std::fopen(path, "wb"))
        {
            std::fprintf(out, "P6\n%d %d\n255\n", window_width, window_height);
            char *p = frame_data.data();
            for (auto i = 0; i < window_width * window_height; ++i)
            {
                std::fputc(*p++, out);
                std::fputc(*p++, out);
                std::fputc(*p++, out);
                ++p;
            }
            std::fclose(out);
        }

        if (++frame_num == total_frames)
            break;
#else
        cur_time += 1.0f / 60;
#endif

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}
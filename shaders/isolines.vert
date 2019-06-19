#version 450 core

layout(location=0) in vec3 position;
layout(location=1) in float noise;

out vertex_data {
    vec3 position;
    float noise;
} vertex_out;

uniform mat4 mvp;

void main(void)
{
    vertex_out.position = position;
    vertex_out.noise = noise;
}

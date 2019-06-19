#version 450 core

layout(triangles) in;
layout(line_strip, max_vertices=2) out;

uniform float threshold;
uniform mat4 mvp;
uniform mat3 normalMatrix;
uniform vec3 lightPosition;

const float ambient = 0.1;

in vertex_data {
    vec3 position;
    float noise;
} vertex_in[];

out float vert_color;

void emit_vertex(int i0, int i1)
{
    float n0 = vertex_in[i0].noise;
    float n1 = vertex_in[i1].noise;

    vec3 v0 = vertex_in[i0].position;
    vec3 v1 = vertex_in[i1].position;

    float t = (threshold - n0)/(n1 - n0);
    vec3 v = normalize(v0 + t*(v1 - v0));

    v += 0.2*(threshold - 0.5)*normalize(v);

    gl_Position = mvp*vec4(v, 1.0);

    vec3 n = normalize(normalMatrix*v);
    vert_color = clamp(ambient + max(dot(n, normalize(lightPosition - v)), 0.0), 0.0, 1.0);

    EmitVertex();
}

void main(void)
{
    bool b0 = vertex_in[0].noise > threshold;
    bool b1 = vertex_in[1].noise > threshold;
    bool b2 = vertex_in[2].noise > threshold;

    if (b0 != b1)
        emit_vertex(0, 1);

    if (b1 != b2)
        emit_vertex(1, 2);

    if (b2 != b0)
        emit_vertex(2, 0);
}

#version 450 core

in float vert_color;

out vec4 frag_color;

void main(void)
{
    frag_color = vec4(vec3(vert_color), 1.0);
}

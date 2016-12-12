#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texture_coord;

out vec2 vertex_texture_coord;

uniform mat4 model_matrix;

uniform float zoom;

void main()
{
    vec2 tex = texture_coord;
    tex *= zoom;
    tex += ((1.0 - zoom) / 2.0);

    vertex_texture_coord = tex;
    gl_Position = model_matrix * vec4(position, 1.0);
}
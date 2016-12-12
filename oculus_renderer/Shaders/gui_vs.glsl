#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texture_coord;

out vec2 vertex_texture_coord;

uniform mat4 model_matrix;

void main()
{
    vertex_texture_coord = texture_coord;
    gl_Position = model_matrix * vec4(position, 1.0);
}
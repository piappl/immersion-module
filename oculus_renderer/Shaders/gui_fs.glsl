#version 330 core

in vec2 vertex_texture_coord;

out vec4 frag_colour;

uniform sampler2D gui_texture;

void main()
{
    vec4 texel = texture(gui_texture, vertex_texture_coord);
    if (texel.a < 0.3)
      discard;
   	frag_colour = texel;
}
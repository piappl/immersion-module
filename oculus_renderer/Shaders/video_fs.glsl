#version 330 core

in vec2 vertex_texture_coord;

out vec4 frag_colour;

uniform bool chromatic_aber_enabled;

uniform sampler2D frame_texture;
uniform sampler2D mask_texture;

uniform float red_scale;
uniform float green_scale;

uniform float red_offset_x;
uniform float red_offset_y;

uniform float green_offset_x;
uniform float green_offset_y;

void main()
{
    vec4 tex = texture(frame_texture, vertex_texture_coord);

	vec4 texel_mask = texture(mask_texture, vertex_texture_coord);
    float channel_offset = texel_mask.r;

    float red_offset = (red_scale - 1) * channel_offset + 1; // linear function
    float green_offset = (green_scale - 1) * channel_offset + 1;

    vec2 red_texture_coords = vertex_texture_coord;
    red_texture_coords *= red_offset;
    red_texture_coords.x += ((1 - red_offset) / 2);
    red_texture_coords.y += ((1 - red_offset) / 2);

    red_texture_coords.x += red_offset_x;
    red_texture_coords.y += red_offset_y;

    vec2 green_coords = vertex_texture_coord;
    green_coords *= green_offset;
    green_coords.x += ((1 - green_offset) / 2);
    green_coords.y += ((1 - green_offset) / 2);

    green_coords.x += green_offset_x;
    green_coords.y += green_offset_y;

    vec4 texel_orginal = texture(frame_texture, vertex_texture_coord);
    vec4 texel_red = texture(frame_texture, red_texture_coords);
    vec4 texel_green = texture(frame_texture, green_coords);

    if (chromatic_aber_enabled)
    {
    	frag_colour = texel_orginal * vec4(0.0, 0.0, 1.0, 1.0) + texel_green * vec4(0.0, 1.0, 0.0, 1.0) + texel_red * vec4(1.0, 0.0, 0.0, 1.0);
	}
	else
	{
    	frag_colour = tex;
	}
}
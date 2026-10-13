#version 330 core

in vec2 uv;
out vec4 color;
uniform sampler2D tex;

void main() { 
    color = vec4(texture(tex, vec2(uv.x, 1.0 - uv.y)).rgb, 1.0); 
}
// Tone curve via a 256x1 LUT texture.

#version 460 core

layout(location = 0) in vec2 vUv;
layout(location = 0) out vec4 fragColor;

uniform sampler2D uInput;
uniform sampler2D uLut; // 256x1, R channel holds the mapped value

void main()
{
    vec4 c = texture(uInput, vUv);
    c.r = texture(uLut, vec2(c.r, 0.5)).r;
    c.g = texture(uLut, vec2(c.g, 0.5)).r;
    c.b = texture(uLut, vec2(c.b, 0.5)).r;
    fragColor = c;
}

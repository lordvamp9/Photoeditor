// Liquify warp: displaces UVs through a displacement texture.

#version 460 core

layout(location = 0) in vec2 vUv;
layout(location = 0) out vec4 fragColor;

uniform sampler2D uInput;
uniform sampler2D uDisplacement; // RG = offset in UV space, centered at 0.5

void main()
{
    vec2 offset = (texture(uDisplacement, vUv).rg - 0.5) * 2.0;
    fragColor = texture(uInput, clamp(vUv + offset, 0.0, 1.0));
}

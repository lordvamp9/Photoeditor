// Layer blend-mode mixing (subset shown; mirrors core/blend_modes.cpp).

#version 460 core

layout(location = 0) in vec2 vUv;
layout(location = 0) out vec4 fragColor;

uniform sampler2D uBase;
uniform sampler2D uBlend;
uniform float uOpacity;
uniform int uMode;

vec3 blendChannel(int mode, vec3 b, vec3 s)
{
    switch (mode) {
    case 1: return min(b, s);                          // Darken
    case 2: return b * s;                              // Multiply
    case 5: return max(b, s);                          // Lighten
    case 6: return b + s - b * s;                      // Screen
    case 9: return mix(2.0 * b * s,                    // Overlay
                       1.0 - 2.0 * (1.0 - b) * (1.0 - s),
                       step(0.5, b));
    case 15: return abs(b - s);                        // Difference
    default: return s;                                 // Normal
    }
}

void main()
{
    vec4 base = texture(uBase, vUv);
    vec4 blend = texture(uBlend, vUv);

    float as = blend.a * uOpacity;
    float ao = as + base.a * (1.0 - as);
    vec3 B = blendChannel(uMode, base.rgb, blend.rgb);
    vec3 Co = (blend.rgb * (1.0 - base.a) + B * base.a) * as + base.rgb * base.a * (1.0 - as);
    fragColor = vec4(ao > 0.0 ? Co / ao : vec3(0.0), ao);
}

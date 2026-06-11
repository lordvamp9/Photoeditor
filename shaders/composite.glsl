// Final composite-to-screen pass: checkerboard + document texture.

#version 460 core

layout(location = 0) in vec2 vUv;
layout(location = 0) out vec4 fragColor;

uniform sampler2D uDocument;
uniform vec2 uViewportSize;
uniform float uCheckerSize;

void main()
{
    vec2 cell = floor(vUv * uViewportSize / uCheckerSize);
    float checker = mod(cell.x + cell.y, 2.0);
    vec3 bg = mix(vec3(0.165, 0.188, 0.227), vec3(0.227, 0.259, 0.306), checker);

    vec4 doc = texture(uDocument, vUv);
    fragColor = vec4(mix(bg, doc.rgb, doc.a), 1.0);
}

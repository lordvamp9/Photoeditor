// Separable Gaussian blur pass (run twice: horizontal then vertical).

#version 460 core

layout(location = 0) in vec2 vUv;
layout(location = 0) out vec4 fragColor;

uniform sampler2D uInput;
uniform vec2 uDirection;   // (1/width, 0) or (0, 1/height)
uniform float uRadius;

void main()
{
    float sigma = max(uRadius / 3.0, 0.5);
    int taps = int(uRadius) * 2 + 1;

    vec4 sum = vec4(0.0);
    float weightSum = 0.0;
    for (int i = -taps / 2; i <= taps / 2; ++i) {
        float w = exp(-float(i * i) / (2.0 * sigma * sigma));
        sum += texture(uInput, vUv + uDirection * float(i)) * w;
        weightSum += w;
    }
    fragColor = sum / weightSum;
}

// Unsharp-mask sharpen pass.

#version 460 core

layout(location = 0) in vec2 vUv;
layout(location = 0) out vec4 fragColor;

uniform sampler2D uInput;
uniform sampler2D uBlurred;
uniform float uAmount;

void main()
{
    vec4 original = texture(uInput, vUv);
    vec4 blurred = texture(uBlurred, vUv);
    vec3 sharpened = original.rgb + (original.rgb - blurred.rgb) * uAmount;
    fragColor = vec4(clamp(sharpened, 0.0, 1.0), original.a);
}

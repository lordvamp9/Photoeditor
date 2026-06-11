// PhotoStudio Premium - shared GLSL helpers (GLSL 4.60)
// These shaders back the (optional) GPU compositing path.

#version 460 core

float luminance(vec3 c)
{
    return dot(c, vec3(0.30, 0.59, 0.11));
}

vec3 clipColor(vec3 c)
{
    float l = luminance(c);
    float n = min(min(c.r, c.g), c.b);
    float x = max(max(c.r, c.g), c.b);
    if (n < 0.0)
        c = l + (c - l) * l / (l - n);
    if (x > 1.0)
        c = l + (c - l) * (1.0 - l) / (x - l);
    return c;
}

vec3 setLuminance(vec3 c, float l)
{
    return clipColor(c + (l - luminance(c)));
}

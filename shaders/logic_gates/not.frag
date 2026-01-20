#version 430 core

out vec4 FragColor;
in vec2 uv;

float sdLine(vec2 p, vec2 a, vec2 b) {
    vec2 pa = p-a, ba = b-a;
    float h = clamp(dot(pa, ba)/dot(ba, ba), 0.0, 1.0);
    return length(pa - ba*h);
}

void main() {
    // NOT gate: triangle + inversion bubble, scaled to [-1, 1]
    float d = sdLine(uv, vec2(-1.0, 1.0), vec2(-1.0, -1.0));
    d = min(d, sdLine(uv, vec2(-1.0, 1.0), vec2(0.6, 0.0)));
    d = min(d, sdLine(uv, vec2(-1.0, -1.0), vec2(0.6, 0.0)));

    // Inversion bubble at right edge
    float dotDist = abs(length(uv - vec2(0.8, 0.0)) - 0.2);

    float aa_d = fwidth(d);
    float aa_dot = fwidth(dotDist);
    float stroke = max(0.03, aa_d * 1.5);
    float stroke_dot = max(0.03, aa_dot * 1.5);

    float bodyMask = 1.0 - smoothstep(stroke - aa_d, stroke + aa_d, d);
    float dotMask = 1.0 - smoothstep(stroke_dot - aa_dot, stroke_dot + aa_dot, dotDist);

    vec3 color = mix(vec3(1.0, 0.0, 0.0), vec3(0.0, 0.0, 1.0), dotMask);
    float alpha = max(bodyMask, dotMask);
    FragColor = vec4(color, alpha);
}

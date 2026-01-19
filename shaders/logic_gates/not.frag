#version 430 core

out vec4 FragColor;
in vec2 uv;

float sdLine(vec2 p, vec2 a, vec2 b) {
    vec2 pa = p-a, ba = b-a;
    float h = clamp(dot(pa, ba)/dot(ba, ba), 0.0, 1.0);
    return length(pa - ba*h);
}

void main() {
    float d = sdLine(uv, vec2(-0.4, 0.4), vec2(-0.4, -0.4));
    d = min(d, sdLine(uv, vec2(-0.4, 0.4), vec2(0.3, 0.0)));
    d = min(d, sdLine(uv, vec2(-0.4, -0.4), vec2(0.3, 0.0)));

    float dotDist = abs(length(uv - vec2(0.45, 0.0)) - 0.1);

    float stroke = 0.01;
    float aa_d = fwidth(d);
    float aa_dot = fwidth(dotDist);

    float bodyMask = 1.0 - smoothstep(stroke - aa_d, stroke + aa_d, d);
    float dotMask = 1.0 - smoothstep(stroke - aa_dot, stroke + aa_dot, dotDist);

    vec3 color = mix(vec3(1.0, 0.0, 0.0), vec3(0.0, 0.0, 1.0), dotMask);
    float alpha = max(bodyMask, dotMask);
    FragColor = vec4(color, alpha);
}

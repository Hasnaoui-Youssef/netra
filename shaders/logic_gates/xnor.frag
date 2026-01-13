#version 430 core

out vec4 FragColor;
in vec2 uv;

float sdBezier(vec2 pos, vec2 p0, vec2 p1, vec2 p2) {
    vec2 a = p1 - p0;
    vec2 b = p0 - 2.0*p1 + p2;
    vec2 c = a * 2.0;
    vec2 d = p0 - pos;
    float kk = 1.0/dot(b,b);
    float kx = kk * dot(a,b);
    float ky = kk * (2.0*dot(a,a)+dot(d,b))/3.0;
    float kz = kk * dot(d,a);
    float res = 0.0;
    float p = ky - kx*kx;
    float p3 = p*p*p;
    float q = kx*(2.0*kx*kx - 3.0*ky) + kz;
    float h = q*q + 4.0*p3;
    if(h >= 0.0) {
        h = sqrt(h);
        vec2 x = (vec2(h,-h)-q)/2.0;
        vec2 uv = sign(x)*pow(abs(x), vec2(1.0/3.0));
        float t = clamp(uv.x+uv.y-kx, 0.0, 1.0);
        res = dot(d+(c+b*t)*t,d+(c+b*t)*t);
    } else {
        float z = sqrt(-p);
        float v = acos(q/(p*z*2.0))/3.0;
        float m = cos(v);
        float n = sin(v)*1.732050808;
        vec3 t = clamp(vec3(m+m,-n-m,n-m)-kx, 0.0, 1.0);
        res = min(dot(d+(c+b*t.x)*t.x,d+(c+b*t.x)*t.x),
                  dot(d+(c+b*t.y)*t.y,d+(c+b*t.y)*t.y));
        res = min(res, dot(d+(c+b*t.z)*t.z,d+(c+b*t.z)*t.z));
    }
    return sqrt(res);
}

void main() {
    float d = sdBezier(uv, vec2(-0.5, 0.5), vec2(-0.2, 0.0), vec2(-0.5, -0.5));
    d = min(d, sdBezier(uv, vec2(-0.5, 0.5), vec2(0.3, 0.5), vec2(0.5, 0.0)));
    d = min(d, sdBezier(uv, vec2(-0.5, -0.5), vec2(0.3, -0.5), vec2(0.5, 0.0)));
    d = min(d, sdBezier(uv, vec2(-0.7, 0.5), vec2(-0.4, 0.0), vec2(-0.7, -0.5)));

    float dotDist = abs(length(uv - vec2(0.62, 0.0)) - 0.1);
    
    float bodyMask = smoothstep(0.02, 0.01, d);
    float dotMask = smoothstep(0.02, 0.01, dotDist);
    
    vec3 color = mix(vec3(1.0, 0.0, 0.0), vec3(0.0, 0.0, 1.0), dotMask);
    float alpha = max(bodyMask, dotMask);
    FragColor = vec4(color, alpha);
}

#version 330 core
in vec3 vLocalPosition;
out vec4 FragColor;
uniform float uDayFactor; // 0 = night, 1 = day
uniform float uTime;

float hash21(vec2 p) {
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}
void main() {
    // Window-facing procedural gradient, no sky texture required.
    float horizon = clamp(vLocalPosition.y + 0.5, 0.0, 1.0);
    vec3 daytime = mix(vec3(0.53, 0.77, 0.93),
                       vec3(0.12, 0.36, 0.77), horizon);
    vec3 nighttime = mix(vec3(0.045, 0.070, 0.17),
                         vec3(0.006, 0.012, 0.055), horizon);
    vec3 color = mix(nighttime, daytime, uDayFactor);
    float x=vLocalPosition.x+.5;
    float ridge=.23+.14*sin(x*6.2)+.08*sin(x*17.0);
    if(horizon<ridge) {
        color=mix(vec3(.018,.033,.066),vec3(.28,.44,.52),uDayFactor);
        if(horizon>ridge-.035) color=mix(vec3(.06,.09,.14),vec3(.83,.86,.82),uDayFactor);
    }
    float foothill=.10+.055*sin(x*11.0+.6);
    if(horizon<foothill) color=mix(vec3(.008,.018,.026),vec3(.16,.28,.27),uDayFactor);

    // A few sparse, stable stars; gentle twinkle only at night.
    vec2 starCell = floor((vLocalPosition.xy + vec2(0.5)) * vec2(37.0, 24.0));
    float chance = hash21(starCell);
    float star = step(0.991, chance);
    float twinkle = 0.70 + 0.30 * sin(uTime * 1.3 + chance * 62.0);
    color += vec3(0.48, 0.61, 0.83) * star * twinkle *
             (1.0 - uDayFactor);
    FragColor = vec4(color, 1.0);
}

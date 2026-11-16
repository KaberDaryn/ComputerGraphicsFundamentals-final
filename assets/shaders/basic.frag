#version 330 core
out vec4 FragColor;
in vec3 vWorldPosition;
in vec3 vWorldNormal;
in vec2 vUV;
in vec4 vLightPosition;

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    float linear;
    float quadratic;
};
uniform PointLight uLights[6];
uniform vec3 uAmbientColor;
uniform vec3 uBaseColor;
uniform vec3 uEmission;
uniform float uShininess;
uniform vec2 uUvScale;
uniform bool uUseTexture;
uniform bool uUnlit;
uniform sampler2D uTexture;
uniform vec3 uViewPosition;
uniform float uOpacity;
uniform sampler2D uShadowMap;
uniform bool uShadowsEnabled;

float visibility(vec3 N, vec3 L) {
    if (!uShadowsEnabled) return 1.0;
    vec3 p = vLightPosition.xyz / vLightPosition.w * 0.5 + 0.5;
    if (p.z > 1.0 || p.z < 0.0) return 1.0;
    float bias = max(0.0012 * (1.0-dot(N,L)),0.00035);
    vec2 stepSize = 1.0 / vec2(textureSize(uShadowMap,0));
    float visible = 0.0;
    for (int x=-1;x<=1;++x) for (int y=-1;y<=1;++y)
        visible += p.z-bias <= texture(uShadowMap,p.xy+vec2(x,y)*stepSize).r ? 1.0 : 0.0;
    return visible / 9.0;
}

void main() {
    vec3 albedo = uBaseColor;
    if (uUseTexture) albedo *= texture(uTexture, vUV * uUvScale).rgb;
    if (uUnlit) { FragColor=vec4(albedo,uOpacity); return; }

    // Based on LearnOpenGL Basic Lighting / Multiple Lights; adapted for
    // our own Blinn-Phong point-light structure, color and attenuation.
    vec3 N = normalize(vWorldNormal);
    vec3 V = normalize(uViewPosition - vWorldPosition);
    vec3 result = uAmbientColor * albedo + uEmission;

    for (int i = 0; i < 6; ++i) {
        if (uLights[i].intensity <= 0.0) continue;
        vec3 direction = uLights[i].position - vWorldPosition;
        float d = length(direction);
        vec3 L = direction / max(d, 0.0001);
        if (i == 0) { L = normalize(uLights[0].position); d = 0.0; }
        float lambert = max(dot(N, L), 0.0);
        vec3 H = normalize(L + V);
        float specular = lambert > 0.0
            ? pow(max(dot(N, H), 0.0), uShininess) : 0.0;
        float attenuation = 1.0 / (1.0 + uLights[i].linear * d +
                                   uLights[i].quadratic * d * d);
        vec3 radiance = uLights[i].color * uLights[i].intensity * attenuation;
        if (i == 0) radiance *= visibility(N,L);
        result += radiance * (lambert * albedo + 0.16 * specular);
    }
    // Simple display-friendly mapping avoids white clipping from strong lights.
    result = result / (result + vec3(1.0));
    FragColor = vec4(pow(result, vec3(1.0 / 2.2)), uOpacity);
}

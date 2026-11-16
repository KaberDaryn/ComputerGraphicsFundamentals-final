#version 330 core
in vec2 vUV;
out vec4 FragColor;
uniform sampler2D uImage;
uniform vec2 uPixel;
uniform bool uGlow;
float luma(vec3 c) { return dot(c,vec3(.299,.587,.114)); }
void main() {
    vec3 c=texture(uImage,vUV).rgb;
    vec3 n=texture(uImage,vUV+vec2(0,uPixel.y)).rgb;
    vec3 s=texture(uImage,vUV-vec2(0,uPixel.y)).rgb;
    vec3 e=texture(uImage,vUV+vec2(uPixel.x,0)).rgb;
    vec3 w=texture(uImage,vUV-vec2(uPixel.x,0)).rgb;
    float range=max(max(luma(n),luma(s)),max(luma(e),luma(w))) -
                min(min(luma(n),luma(s)),min(luma(e),luma(w)));
    // Conservative edge smoothing, without blurring textile and wood grain.
    if (range>.10) c=mix(c,(n+s+e+w)*.25,.30);
    if (uGlow) {
        vec3 bloom=vec3(0);
        for(int x=-2;x<=2;++x) for(int y=-2;y<=2;++y) {
            vec3 b=texture(uImage,vUV+vec2(x,y)*uPixel*2.0).rgb;
            bloom+=max(b-vec3(.76),vec3(0));
        }
        c+=bloom*(.35/25.0);
    }
    FragColor=vec4(c,1);
}

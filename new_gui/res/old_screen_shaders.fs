#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform vec2 resolution;
uniform float time;

uniform sampler2D texture0;

out vec4 final_color;

vec2 curve_screen(vec2 uv) {
    uv = uv * 2.0 - 1.0;

    vec2 offset = uv.yx / 4.0;
    uv = uv + uv * offset * offset;

    uv = uv * 0.5 + 0.5;
    return uv;
}

void main() {
    vec2 uv = fragTexCoord;
    vec2 curved_uv = curve_screen(uv);
    vec2 split = vec2(0.0015, 0.0);
    float number_line = resolution.y / 2;
    float speed = 10.0;
    float scan_speed = 2.0;

    if (curved_uv.x < 0.0 || curved_uv.x > 1.0 || curved_uv.y < 0.0 || curved_uv.y > 1.0) {
        final_color = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }
    // final_color = texture(texture0, curved_uv);
    final_color.r = texture(texture0, curved_uv - split).r;
    final_color.g = texture(texture0, curved_uv).g;
    final_color.b = texture(texture0, curved_uv + split).b;
    // final_color.rgb *= distance(curved_uv, vec2(-0.5));
    float dist = distance(curved_uv, vec2(0.5));
    float vignette = smoothstep(0.85, 0.4, dist);
    final_color.rgb *= vignette;

    float scanline =  0.8 + 0.1 * sin(curved_uv.y * number_line * 3.1415 + time * speed);
    float scan_bar =  0.9 + 0.1 * sin(curved_uv.y * 4.0 + time * scan_speed);
    final_color.rgb *= scanline * scan_bar;
}

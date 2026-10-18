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
    if (curved_uv.x < 0.0 || curved_uv.x > 1.0 || curved_uv.y < 0.0 || curved_uv.y > 1.0) {
        final_color = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }
    final_color = texture(texture0, curved_uv);
}

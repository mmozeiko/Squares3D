//
uniform sampler2D tex_small;
uniform float tex_small_size;
uniform float divider;

void main(void)
{
    vec2 hdr_pos = gl_TexCoord[0].xy;

    float offset1 = 1.0/tex_small_size;
    float offset2 = 2.0/tex_small_size;
 
    // 5x5 Gauss blur

    vec4 color = 15.0 * texture2D(tex_small, hdr_pos);
    color += 2.0 * texture2D(tex_small, hdr_pos + vec2(-offset2, -offset2));
    color += 4.0 * texture2D(tex_small, hdr_pos + vec2(-offset1, -offset2));
    color += 5.0 * texture2D(tex_small, hdr_pos + vec2(0.0, -offset2));
    color += 4.0 * texture2D(tex_small, hdr_pos + vec2(offset1, -offset2));
    color += 2.0 * texture2D(tex_small, hdr_pos + vec2(offset2, -offset2));
    color += 4.0 * texture2D(tex_small, hdr_pos + vec2(-offset2, -offset1));
    color += 9.0 * texture2D(tex_small, hdr_pos + vec2(-offset1, -offset1));
    color += 12.0 * texture2D(tex_small, hdr_pos + vec2(0.0, -offset1));
    color += 9.0 * texture2D(tex_small, hdr_pos + vec2(+offset1, -offset1));
    color += 4.0 * texture2D(tex_small, hdr_pos + vec2(+offset2, -offset1));
    color += 5.0 * texture2D(tex_small, hdr_pos + vec2(-offset2, 0.0));
    color += 12.0 * texture2D(tex_small, hdr_pos + vec2(-offset1, 0.0));
    color += 12.0 * texture2D(tex_small, hdr_pos + vec2(+offset1, 0.0));
    color += 5.0 * texture2D(tex_small, hdr_pos + vec2(+offset2, 0.0));
    color += 4.0 * texture2D(tex_small, hdr_pos + vec2(-offset2, +offset1));
    color += 9.0 * texture2D(tex_small, hdr_pos + vec2(-offset1, +offset1));
    color += 12.0 * texture2D(tex_small, hdr_pos + vec2(0.0, +offset1));
    color += 9.0 * texture2D(tex_small, hdr_pos + vec2(+offset1, +offset1));
    color += 4.0 * texture2D(tex_small, hdr_pos + vec2(+offset2, +offset1));
    color += 2.0 * texture2D(tex_small, hdr_pos + vec2(-offset2, +offset2));
    color += 4.0 * texture2D(tex_small, hdr_pos + vec2(-offset1, +offset2));
    color += 5.0 * texture2D(tex_small, hdr_pos + vec2(0.0, +offset2));
    color += 4.0 * texture2D(tex_small, hdr_pos + vec2(+offset1, +offset2));
    color += 2.0 * texture2D(tex_small, hdr_pos + vec2(+offset2, +offset2));
    
    gl_FragColor = color / divider;
}

//
uniform sampler2D tex_small;
uniform float tex_small_size;
uniform vec4 tex_offset;

void main(void)
{
    vec2 hdr_pos = gl_TexCoord[0].xy;

    vec2 offset1 = vec2(tex_offset);
    vec2 offset2 = 2.0*offset1;
    vec2 offset3 = 3.0*offset1;
    vec2 offset4 = 4.0*offset1;
    vec2 offset5 = 5.0*offset1;
    vec2 offset6 = 6.0*offset1;
    vec2 offset7 = 7.0*offset1;
    vec2 offset8 = 8.0*offset1;
 
    vec4 color = 1.0000 * texture2D(tex_small, hdr_pos) +
                 0.9394 * texture2D(tex_small, hdr_pos + offset1) +
                 0.9394 * texture2D(tex_small, hdr_pos - offset1) +
                 0.7788 * texture2D(tex_small, hdr_pos + offset2) +
                 0.7788 * texture2D(tex_small, hdr_pos - offset2) +
                 0.5697 * texture2D(tex_small, hdr_pos + offset3) +
                 0.5697 * texture2D(tex_small, hdr_pos - offset3) +
                 0.3678 * texture2D(tex_small, hdr_pos + offset4) +
                 0.3678 * texture2D(tex_small, hdr_pos - offset4) +
                 0.2096 * texture2D(tex_small, hdr_pos + offset5) +
                 0.2096 * texture2D(tex_small, hdr_pos - offset5) +
                 0.1053 * texture2D(tex_small, hdr_pos + offset6) +
                 0.1053 * texture2D(tex_small, hdr_pos - offset6) +
                 0.0467 * texture2D(tex_small, hdr_pos + offset7) +
                 0.0467 * texture2D(tex_small, hdr_pos - offset7) +
                 0.0183 * texture2D(tex_small, hdr_pos + offset8) +
                 0.0183 * texture2D(tex_small, hdr_pos - offset8);

    gl_FragColor = color / (1.0 + 2.0*3.0359);
}

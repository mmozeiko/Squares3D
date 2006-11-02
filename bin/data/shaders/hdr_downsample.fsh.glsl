//
uniform sampler2D tex_source;
const float hdr_eps = 0.86;

void main(void)
{
    vec4 srcpixel = texture2D(tex_source, gl_TexCoord[0].xy);
    if (0.3 * srcpixel.r + 0.59 * srcpixel.g + 0.11 * srcpixel.b < hdr_eps)
    {
        srcpixel = vec4(0, 0, 0, 0);
    }
    
    gl_FragColor = srcpixel;
}

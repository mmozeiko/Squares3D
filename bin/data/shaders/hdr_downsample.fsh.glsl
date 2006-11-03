//
uniform sampler2D tex_source;
uniform float hdr_eps;
uniform vec4 hdr_mul;

void main(void)
{
    vec4 srcpixel = texture2D(tex_source, gl_TexCoord[0].xy);
    if (0.3 * srcpixel.r + 0.59 * srcpixel.g + 0.11 * srcpixel.b < hdr_eps)
    {
        srcpixel = vec4(0, 0, 0, 0);
    }
    
    gl_FragColor = hdr_mul * srcpixel;
}

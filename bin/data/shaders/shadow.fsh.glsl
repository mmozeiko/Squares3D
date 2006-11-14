//
uniform sampler2D tex_source;
uniform sampler2DShadow tex_shadow;
varying vec4 light_diffuse;

void main(void)
{
    vec4 pix_source = texture2D(tex_source, gl_TexCoord[0].st);
    vec4 pix_shadow = shadow2DProj(tex_shadow, gl_TexCoord[1]);
    vec4 tmp = max(vec4(0.2, 0.2, 0.2, 0.2), pix_shadow);
    if (any(lessThanEqual(gl_TexCoord[1].st, vec2(0.0, 0.0))) ||
        any(greaterThanEqual(gl_TexCoord[1].st, vec2(1.0, 1.0))))
    {
        tmp = vec4(1.0, 1.0, 1.0, 1.0);
    }
    gl_FragColor = (gl_LightSource[1].ambient + light_diffuse * tmp) * pix_source;
}

//
uniform sampler2D tex_source;
uniform sampler2DShadow tex_shadow;
varying vec4 light_diffuse;

void main(void)
{
    vec4 pix_source = texture2D(tex_source, gl_TexCoord[0].st);
    vec4 pix_shadow = shadow2DProj(tex_shadow, gl_TexCoord[1]);
    vec3 tmp = max(vec3(0.2, 0.2, 0.2), pix_shadow.rgb);
    if (any(lessThan(gl_TexCoord[1].st, vec2(0.0, 0.0))) ||
        any(greaterThan(gl_TexCoord[1].st, vec2(1.0, 1.0))))
    {
        tmp = vec3(1.0, 1.0, 1.0);
    }
    vec4 result;
    result.rgb = (gl_LightSource[1].ambient.rgb + light_diffuse.rgb * tmp) * pix_source.rgb;
    result.a = pix_source.a;
    gl_FragColor = result;
}

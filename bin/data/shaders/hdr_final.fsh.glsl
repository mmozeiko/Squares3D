//
uniform sampler2D tex_source;
uniform sampler2D tex_blur;

vec4 getBlurred(vec2 pos)
{
    vec4 total = vec4(0,0,0,0); //texture2D(tex_blur, pos);

    {vec4 p = texture2D(tex_blur, pos + vec2(1.0/512.0, 0.0/512.0));
    if (p.r > 0.8 || p.g > 0.8 || p.b > 0.8) total += p;}

    {vec4 p = texture2D(tex_blur, pos + vec2(-1.0/512.0, 0.0/512.0));
    if (p.r > 0.8 || p.g > 0.8 || p.b > 0.8) total += p;}

    {vec4 p = texture2D(tex_blur, pos + vec2(0.0/512.0, 1.0/512.0));
    if (p.r > 0.8 || p.g > 0.8 || p.b > 0.8) total += p;}

    {vec4 p = texture2D(tex_blur, pos + vec2(0.0/512.0, -1.0/512.0));
    if (p.r > 0.8 || p.g > 0.8 || p.b > 0.8) total += p;}

    {vec4 p = texture2D(tex_blur, pos + vec2(0.0/512.0, -2.0/512.0));
    if (p.r > 0.8 || p.g > 0.8 || p.b > 0.8) total += p;}

    {vec4 p = texture2D(tex_blur, pos + vec2(0.0/512.0, +2.0/512.0));
    if (p.r > 0.8 || p.g > 0.8 || p.b > 0.8) total += p;}

    {vec4 p = texture2D(tex_blur, pos + vec2(2.0/512.0, 0.0/512.0));
    if (p.r > 0.8 || p.g > 0.8 || p.b > 0.8) total += p;}

    {vec4 p = texture2D(tex_blur, pos + vec2(-2.0/512.0, 0.0/512.0));
    if (p.r > 0.8 || p.g > 0.8 || p.b > 0.8) total += p;}

    {vec4 p = texture2D(tex_blur, pos + vec2(0.0/512.0, -3.0/512.0));
    if (p.r > 0.8 || p.g > 0.8 || p.b > 0.8) total += p;}

    {vec4 p = texture2D(tex_blur, pos + vec2(0.0/512.0, +3.0/512.0));
    if (p.r > 0.8 || p.g > 0.8 || p.b > 0.8) total += p;}

    {vec4 p = texture2D(tex_blur, pos + vec2(3.0/512.0, 0.0/512.0));
    if (p.r > 0.8 || p.g > 0.8 || p.b > 0.8) total += p;}

    {vec4 p = texture2D(tex_blur, pos + vec2(-3.0/512.0, 0.0/512.0));
    if (p.r > 0.8 || p.g > 0.8 || p.b > 0.8) total += p;}

    return 0.7 * total / (1+3*4);
}

void main(void)
{
    vec4 pix_source = texture2D(tex_source, gl_TexCoord[0].xy);
    vec4 pix_blur = getBlurred(gl_TexCoord[0].xy);
    gl_FragColor = pix_source + pix_blur;
}

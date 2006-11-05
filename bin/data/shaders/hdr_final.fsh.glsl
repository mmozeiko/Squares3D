//
uniform sampler2D tex_small_64;
uniform sampler2D tex_small_128;
uniform sampler2D tex_small_256;
uniform sampler2D tex_source;

const float hdr_exposure = -1.3;
void main(void)
{
    vec4 pix_source = texture2D(tex_source, gl_TexCoord[0].xy);

    vec4 pix_blur = 
                    texture2D(tex_small_256, gl_TexCoord[1].xy)
                  + texture2D(tex_small_128, gl_TexCoord[1].xy)
                  + texture2D(tex_small_64,  gl_TexCoord[1].xy);
//    gl_FragColor = (1.0 - exp((pix_source+pix_blur) * hdr_exposure));
    gl_FragColor = pix_source + pix_blur;
}

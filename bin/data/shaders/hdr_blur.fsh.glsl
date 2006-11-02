//

//#extension ARB_texture_rectangle : require

uniform sampler2D tex_hdr;
uniform vec4 blur_offset;

const int samples = 8;
const float sum = 3.0359;

const float eps = 0.8;

void main(void)
{
    // Points on a Gauss curve, used as weights for the blur filter:
    float weights[9];
    weights[0] = 1.0000;
    weights[1] = 0.9394;
    weights[2] = 0.7788;
    weights[3] = 0.5697;
    weights[4] = 0.3678;
    weights[4] = 0.2096;
    weights[5] = 0.1053;
    weights[7] = 0.0467;
    weights[8] = 0.0183;

    // Sample the center pixel:

    vec4 p = vec4(gl_TexCoord[0].x, gl_TexCoord[0].y, 0, 0);
    vec4 color = texture2D(tex_hdr, vec2(p)) * weights[0];
    //if (color.r<eps && color.g<eps && color.b<eps)
    //{
    //    color = vec4(0, 0, 0, 0);
    //}
      

    vec2 tc;
    vec2 offset;

    // Sample away from the center in both directions:
    for (int i = 1; i <= samples; i++)
    {
        // blur_offset determines whether this is a horizontal or vertical blur.
        offset = blur_offset.xy * float(i);

        // Sample on one side of the center:
        tc = gl_TexCoord[0].xy + offset;

        vec4 tmp = texture2D(tex_hdr, tc);
        //if (tmp.r>=eps || tmp.g>=eps || tmp.b>=eps)
        //{
            color += tmp * weights[i];
        //}

        // Sample on the other side of the center:
        tc = gl_TexCoord[0].xy - offset;

        tmp = texture2D(tex_hdr, tc);
        //if (tmp.r>=eps || tmp.g>=eps || tmp.b>=eps)
        //{
            color += tmp * weights[i];
        //}
    }

    // Scale down the summed samples to get the final color.
    gl_FragColor = color / (1.0 + 2.0*sum);
}


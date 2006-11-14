//
varying vec4 light_diffuse;

void main(void)
{
    vec4 ecPosition  = gl_ModelViewMatrix * gl_Vertex;
    vec4 shadowCoord;
    shadowCoord.s = dot( ecPosition, gl_EyePlaneS[1] );
    shadowCoord.t = dot( ecPosition, gl_EyePlaneT[1] );
    shadowCoord.p = dot( ecPosition, gl_EyePlaneR[1] );
    shadowCoord.q = dot( ecPosition, gl_EyePlaneQ[1] );
    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_TexCoord[1] = shadowCoord;

    gl_Position = gl_ProjectionMatrix * ecPosition;

    vec3 normal = gl_NormalMatrix * gl_Normal;
    vec3 lightVec = normalize(gl_LightSource[1].position.xyz - ecPosition.xyz);
    float nxDir = max(0.0, dot(normal, lightVec));
    light_diffuse = gl_LightSource[1].diffuse * nxDir;
}

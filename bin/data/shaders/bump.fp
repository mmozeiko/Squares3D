//

uniform sampler2D decalMap;
uniform sampler2D heightMap;

varying vec2 uv;
varying vec3 lightVec;
varying vec3 halfVec;

const float diffuseCoeff = 0.8;
const float specularCoeff = 0.15;

void main()
{
   vec3 pos = lightVec; //normalize(gl_LightSource[0].position.xyz); // light vector in object space
   vec3 hhalf = halfVec; //vec3(1,0,0); // eye vector in object space

   vec2 uv = gl_TexCoord[0].st;
   
   vec3 decalColor = texture2D(decalMap, uv).rgb;
   vec3 heightColor = texture2D(heightMap, uv).rgb;

   vec3 normal = 2.0 * (heightColor - 0.5);
   normal = normalize(normal);
   float diffuse = max(dot(pos, normal), 0.0) * diffuseCoeff;

   float specular = max(dot(hhalf, normal), 0.0);
   specular = pow(specular, 128.0) * specularCoeff;
   
   gl_FragColor = vec4(vec3(diffuse) * decalColor + vec3(specular), 1.0);
}

/*

!!ARBfp1.0
ATTRIB detailCoords = fragment.texcoord[0];
ATTRIB bumpCoords   = fragment.texcoord[1];
ATTRIB ambientColor = fragment.color.primary;
PARAM lightVector  = { 0, 1, 0, 1 }; #state.light[0].position;
OUTPUT outColor = result.color;

TEMP detailColor, ligthVectorFinal, bumpNormalVectorFinal, diffuse, tmp;

TXP detailColor, detailCoords, texture[0], 2D;

SUB ligthVectorFinal, lightVector, {0.5, 0.5, 0.5, 0.5};
MUL ligthVectorFinal, ligthVectorFinal, {2,2,2,2};

TXP tmp, bumpCoords, texture[1], 2D;
SUB tmp, tmp, {0.5, 0.5, 0.5, 0.5};
MUL bumpNormalVectorFinal, tmp, {2, 2, 2, 2};

DP3 diffuse, bumpNormalVectorFinal, ligthVectorFinal; 

MUL tmp, diffuse, detailColor;
ADD tmp, tmp, ambientColor;
MOV outColor.xyz, tmp;
MOV outColor.w, 1.0;

END


#ATTRIB tex0 = fragment.texcoord[0];
#ATTRIB tex1 = fragment.texcoord[1];
#ATTRIB col = fragment.color.primary;
#ATTRIB pos = fragment.position;
##ATTRIB Lpos = state.light[0].position;
#OUTPUT outColor = result.color;
#
#TEMP tmp, tmp0, tmp1;
#TXP tmp0, tex0, texture[0], 2D;
##TXP tmp1, tex1, texture[1], 2D;
#MUL  outColor, tmp0, col;
#
##SUB tmp, Lpos, pos;
##DT3 tmp, tmp, tmp1;
#
##MAD outColor, tmp, tmp0, col;




#        float3 detailColor = tex2D(detailTexture, detailCoords).rgb;
#                                 #state.light[n].position 
#        // Uncompress vectors ([0, 1] -> [-1, 1])
#        float3 lightVectorFinal = 2.0 * (lightVector.rgb - 0.5);
#        float3 bumpNormalVectorFinal = 2.0 * (tex2D(bumpTexture, bumpCoords).rgb - 0.5);
#
#        // Compute diffuse factor
#        float diffuse = dot(bumpNormalVectorFinal, lightVectorFinal);
#
#        return float4(diffuse * detailColor + ambientColor, 1.0);

*/

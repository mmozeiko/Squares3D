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

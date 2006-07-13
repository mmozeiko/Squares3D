!!ARBfp1.0
#ATTRIB tex = fragment.texcoord;
#ATTRIB col = fragment.color.primary;
#OUTPUT outColor = result.color;

#TEMP tmp;
#TXP tmp, tex, texture[0], 2D;
#MUL outColor, tmp, col;

PARAM blue = { 0.0, 0.0, 1.0, 1.0 };
MOV result.color, blue;

END

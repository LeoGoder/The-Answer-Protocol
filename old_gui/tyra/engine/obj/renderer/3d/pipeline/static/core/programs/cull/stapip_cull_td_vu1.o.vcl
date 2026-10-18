
.name StaPipVU1CullTD
.syntax new
.name StaPipVU1Cull_TD
.vu
.init_vf_all
.init_vi_all
--enter
--endenter

   fcset   0x000000


   lq             gifSetTag, 19(vi00)


   lq             mvp[0], 0+0(vi00)
   lq             mvp[1], 0+1(vi00)
   lq             mvp[2], 0+2(vi00)
   lq             mvp[3], 0+3(vi00)


   lq.xyz      lightMatrix[0],       4+0(vi00)
   lq.xyz      lightMatrix[1],       4+1(vi00)
   lq.xyz      lightMatrix[2],       4+2(vi00)
   lq.xyz      lightDirections[0],   12(vi00)
   lq.xyz      lightDirections[1],   12+1(vi00)
   lq.xyz      lightDirections[2],   12+2(vi00)
   lq.xyz      lightColors[0],      15(vi00)
   lq.xyz      lightColors[1],      15+1(vi00)
   lq.xyz      lightColors[2],      15+2(vi00)
   lq.xyz      ambientColor,         15+3(vi00)


   lq      lodGifTag,             9(vi00)
   lq      testsTag,           10(vi00)
   lq      texBufferClutGifTag,   11(vi00)

begin:
    xtop buffer

   lq.xyz  scale,                  0(buffer)
   lq      primTag,                1(buffer)

    iaddiu  vertexData,         buffer,         2
    ilw.w   vertexCount,        0(buffer)
    iadd    stqData,            vertexData,     vertexCount
    iadd    normalData,         stqData,        vertexCount
    iadd    kickAddress,        normalData,     vertexCount
    iadd    destAddress,        normalData,     vertexCount

   sq gifSetTag,            0(destAddress)
   sq testsTag,             1(destAddress)
   sq gifSetTag,            2(destAddress)
   sq lodGifTag,            3(destAddress)
   sq gifSetTag,            4(destAddress)
   sq texBufferClutGifTag,  5(destAddress)
   sq primTag,              6(destAddress)
   iaddiu                     destAddress,    destAddress,    7

    iadd vertexCounter, buffer, vertexCount
vertexLoop:
        lq      vertex1,  (vertexData)
        lq      stq1,     (stqData)
        lq.xyz  normal1,  (normalData) 
        lq      vertex2,  1(vertexData)
        lq      stq2,     1(stqData)
        lq.xyz  normal2,  1(normalData) 
        lq      vertex3,  2(vertexData)
        lq      stq3,     2(stqData)
        lq.xyz  normal3,  2(normalData) 

   mul            acc,           mvp[0], vertex1[x]
   madd           acc,           mvp[1], vertex1[y]
   madd           acc,           mvp[2], vertex1[z]
   madd           vertex1, mvp[3], vertex1[w]


   clipw.xyz	vertex1,   vertex1	
   fcand       VI01,       0x3FFFF
   iaddiu      adcBit,     VI01, 0x7FFF
   isw.w       adcBit,     2(destAddress)


   div            q, vf00[w], vertex1[w]
   mul.xyz        vertex1, vertex1, q


   mula.xyz    acc,        scale,    vf00[w]
   madd.xyz    vertex1,   vertex1,   scale
   ftoi4.xyz   vertex1,   vertex1


   mulq  outputStq1,  stq1,  q


   mul.xyz     acc,              lightMatrix[0],       normal1[x]	
   madd.xyz    acc,              lightMatrix[1],       normal1[y]
   madd.xyz    normal1,         lightMatrix[2],       normal1[z]
   mula.xyz    acc,              lightDirections[0],   normal1[x]
   madd.xyz    acc,              lightDirections[1],   normal1[y]
   madd.xyz    outputColor1,    lightDirections[2],   normal1[z]
   mini.xyz    outputColor1,    outputColor1,          vf00[w]
   max.xyz     outputColor1,    outputColor1,          vf00[x]
   mula.xyz    acc,              lightColors[0],       outputColor1[x]
   madda.xyz   acc,              lightColors[1],       outputColor1[y]
   madda.xyz   acc,              lightColors[2],       outputColor1[z]
   madd.xyz    outputColor1,    ambientColor,         vf00[w]
   loi         128
	addi.w      outputColor1,    vf00,    i


   loi         255
   mini.xyz    outputColor1, outputColor1, i
   max.xyz     outputColor1, outputColor1, vf00[x]
   ftoi0       outputColor1, outputColor1


   mul            acc,           mvp[0], vertex2[x]
   madd           acc,           mvp[1], vertex2[y]
   madd           acc,           mvp[2], vertex2[z]
   madd           vertex2, mvp[3], vertex2[w]


   clipw.xyz	vertex2,   vertex2	
   fcand       VI01,       0x3FFFF
   iaddiu      adcBit,     VI01, 0x7FFF
   isw.w       adcBit,     2+3(destAddress)


   div            q, vf00[w], vertex2[w]
   mul.xyz        vertex2, vertex2, q


   mula.xyz    acc,        scale,    vf00[w]
   madd.xyz    vertex2,   vertex2,   scale
   ftoi4.xyz   vertex2,   vertex2


   mulq  outputStq2,  stq2,  q


   mul.xyz     acc,              lightMatrix[0],       normal2[x]	
   madd.xyz    acc,              lightMatrix[1],       normal2[y]
   madd.xyz    normal2,         lightMatrix[2],       normal2[z]
   mula.xyz    acc,              lightDirections[0],   normal2[x]
   madd.xyz    acc,              lightDirections[1],   normal2[y]
   madd.xyz    outputColor2,    lightDirections[2],   normal2[z]
   mini.xyz    outputColor2,    outputColor2,          vf00[w]
   max.xyz     outputColor2,    outputColor2,          vf00[x]
   mula.xyz    acc,              lightColors[0],       outputColor2[x]
   madda.xyz   acc,              lightColors[1],       outputColor2[y]
   madda.xyz   acc,              lightColors[2],       outputColor2[z]
   madd.xyz    outputColor2,    ambientColor,         vf00[w]
   loi         128
	addi.w      outputColor2,    vf00,    i


   loi         255
   mini.xyz    outputColor2, outputColor2, i
   max.xyz     outputColor2, outputColor2, vf00[x]
   ftoi0       outputColor2, outputColor2


   mul            acc,           mvp[0], vertex3[x]
   madd           acc,           mvp[1], vertex3[y]
   madd           acc,           mvp[2], vertex3[z]
   madd           vertex3, mvp[3], vertex3[w]


   clipw.xyz	vertex3,   vertex3	
   fcand       VI01,       0x3FFFF
   iaddiu      adcBit,     VI01, 0x7FFF
   isw.w       adcBit,     2+6(destAddress)


   div            q, vf00[w], vertex3[w]
   mul.xyz        vertex3, vertex3, q


   mula.xyz    acc,        scale,    vf00[w]
   madd.xyz    vertex3,   vertex3,   scale
   ftoi4.xyz   vertex3,   vertex3


   mulq  outputStq3,  stq3,  q


   mul.xyz     acc,              lightMatrix[0],       normal3[x]	
   madd.xyz    acc,              lightMatrix[1],       normal3[y]
   madd.xyz    normal3,         lightMatrix[2],       normal3[z]
   mula.xyz    acc,              lightDirections[0],   normal3[x]
   madd.xyz    acc,              lightDirections[1],   normal3[y]
   madd.xyz    outputColor3,    lightDirections[2],   normal3[z]
   mini.xyz    outputColor3,    outputColor3,          vf00[w]
   max.xyz     outputColor3,    outputColor3,          vf00[x]
   mula.xyz    acc,              lightColors[0],       outputColor3[x]
   madda.xyz   acc,              lightColors[1],       outputColor3[y]
   madda.xyz   acc,              lightColors[2],       outputColor3[z]
   madd.xyz    outputColor3,    ambientColor,         vf00[w]
   loi         128
	addi.w      outputColor3,    vf00,    i


   loi         255
   mini.xyz    outputColor3, outputColor3, i
   max.xyz     outputColor3, outputColor3, vf00[x]
   ftoi0       outputColor3, outputColor3

        sq      outputStq1,     0(destAddress)
        sq      outputColor1,   1(destAddress)
        sq.xyz  vertex1,        2(destAddress)
        sq      outputStq2,     0+3(destAddress)
        sq      outputColor2,   1+3(destAddress)
        sq.xyz  vertex2,        2+3(destAddress)
        sq      outputStq3,     0+6(destAddress)
        sq      outputColor3,   1+6(destAddress)
        sq.xyz  vertex3,        2+6(destAddress)
        iaddiu  vertexData,     vertexData,     3                         
        iaddiu  stqData,        stqData,        3  
        iaddiu  normalData,     normalData,     3  
        iaddiu  destAddress,    destAddress,    9
        iaddi   vertexCounter,  vertexCounter,  -3  
        ibne    vertexCounter,  buffer, vertexLoop  
    xgkick kickAddress 
--barrier 
--cont
    b   begin
--exit
--endexit

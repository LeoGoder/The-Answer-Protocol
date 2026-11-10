
.name DynPipVU1TD
.syntax new
.name DynPipVU1_TD
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


   lq      lodGifTag,             9(vi00)
   lq      testsTag,           10(vi00)
   lq      texBufferClutGifTag,   11(vi00)

begin:
    xtop buffer

   lq      primTag,   1(buffer)

    ilw.w   vertexCount,        0(buffer)
    iaddiu  vertexDataFrom,     buffer,         2
    iadd    stqDataFrom,        vertexDataFrom, vertexCount
    iadd    stqDataFrom,        stqDataFrom,    vertexCount
    iadd    normalDataFrom,     stqDataFrom,    vertexCount
    iadd    normalDataFrom,     normalDataFrom, vertexCount
    iadd    kickAddress,        normalDataFrom, vertexCount
    iadd    destAddress,        kickAddress,    vertexCount
    iadd    kickAddress,        kickAddress,    vertexCount

   sq gifSetTag,            0(destAddress)
   sq testsTag,             1(destAddress)
   sq gifSetTag,            2(destAddress)
   sq lodGifTag,            3(destAddress)
   sq gifSetTag,            4(destAddress)
   sq texBufferClutGifTag,  5(destAddress)
   sq primTag,              6(destAddress)
   iaddiu                     destAddress,    destAddress,    7


   lq.y  interp, 8(vi00)


   lq.xyz  scale,  0(buffer)

    iadd vertexCounter, buffer, vertexCount
vertexLoop:
        iadd    vertexDataTo,   vertexDataFrom, vertexCount
        iadd    stqDataTo,      stqDataFrom,    vertexCount
        iadd    normalDataTo,   normalDataFrom, vertexCount
        lq      vertex1From,    (vertexDataFrom)
        lq      vertex1To,      (vertexDataTo)

   sub   temp1,      vertex1To,    vertex1From
   mul   temp2,      temp1,   interp[y]
   add   vertex1,   temp2,   vertex1From

        lq      stq1From,       (stqDataFrom)
        lq      stq1To,         (stqDataTo)

   sub   temp1,      stq1To,    stq1From
   mul   temp2,      temp1,   interp[y]
   add   stq1,   temp2,   stq1From

        lq.xyz  normal1From,    (normalDataFrom)
        lq.xyz  normal1To,      (normalDataTo) 

   sub.xyz   temp1,      normal1To,    normal1From
   mul.xyz   temp2,      temp1,   interp[y]
   add.xyz   normal1,   temp2,   normal1From


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

        sq      outputStq1,     0(destAddress)
        sq      outputColor1,   1(destAddress)
        sq.xyz  vertex1,        2(destAddress)
        lq      vertex2From,    1(vertexDataFrom)
        lq      vertex2To,      1(vertexDataTo)

   sub   temp1,      vertex2To,    vertex2From
   mul   temp2,      temp1,   interp[y]
   add   vertex2,   temp2,   vertex2From

        lq      stq2From,       1(stqDataFrom)
        lq      stq2To,         1(stqDataTo)

   sub   temp1,      stq2To,    stq2From
   mul   temp2,      temp1,   interp[y]
   add   stq2,   temp2,   stq2From

        lq.xyz  normal2From,    1(normalDataFrom) 
        lq.xyz  normal2To,      1(normalDataTo) 

   sub.xyz   temp1,      normal2To,    normal2From
   mul.xyz   temp2,      temp1,   interp[y]
   add.xyz   normal2,   temp2,   normal2From


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

        sq      outputStq2,     0+3(destAddress)
        sq      outputColor2,   1+3(destAddress)
        sq.xyz  vertex2,        2+3(destAddress)
        lq      vertex3From,    2(vertexDataFrom)
        lq      vertex3To,      2(vertexDataTo)

   sub   temp1,      vertex3To,    vertex3From
   mul   temp2,      temp1,   interp[y]
   add   vertex3,   temp2,   vertex3From

        lq      stq3From,       2(stqDataFrom)
        lq      stq3To,         2(stqDataTo)

   sub   temp1,      stq3To,    stq3From
   mul   temp2,      temp1,   interp[y]
   add   stq3,   temp2,   stq3From

        lq.xyz  normal3From,    2(normalDataFrom) 
        lq.xyz  normal3To,      2(normalDataTo) 

   sub.xyz   temp1,      normal3To,    normal3From
   mul.xyz   temp2,      temp1,   interp[y]
   add.xyz   normal3,   temp2,   normal3From


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

        sq      outputStq3,     0+6(destAddress)
        sq      outputColor3,   1+6(destAddress)
        sq.xyz  vertex3,        2+6(destAddress)
        iaddiu  vertexDataFrom, vertexDataFrom, 3
        iaddiu  stqDataFrom,    stqDataFrom,    3  
        iaddiu  normalDataFrom, normalDataFrom, 3  
        iaddiu  destAddress,    destAddress,    9
        iaddi   vertexCounter,  vertexCounter,  -3  
        ibne    vertexCounter,  buffer, vertexLoop  
    xgkick kickAddress 
--barrier 
--cont
    b   begin
--exit
--endexit

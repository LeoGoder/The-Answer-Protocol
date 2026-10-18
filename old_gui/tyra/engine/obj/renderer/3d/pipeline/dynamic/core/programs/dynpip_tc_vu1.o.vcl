
.name DynPipVU1TC
.syntax new
.name DynPipVU1_TC
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


   lq       singleColor,          7(vi00)
   ilw.x    singleColorEnabled,   8(vi00)


   lq      lodGifTag,             9(vi00)
   lq      testsTag,           10(vi00)
   lq      texBufferClutGifTag,   11(vi00)


   loi         255
   mini.xyz    singleColor, singleColor, i
   max.xyz     singleColor, singleColor, vf00[x]
   ftoi0       singleColor, singleColor

begin:
    xtop buffer

   lq      primTag,   1(buffer)

    ilw.w   vertexCount,        0(buffer)
    iaddiu  vertexDataFrom,     buffer,         2
    iadd    stqDataFrom,        vertexDataFrom, vertexCount
    iadd    stqDataFrom,        stqDataFrom,    vertexCount
    iadd    kickAddress,        stqDataFrom,    vertexCount
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

        sq      outputStq1,     0(destAddress)
        sq      singleColor,    1(destAddress)
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

        sq      outputStq2,     0+3(destAddress)
        sq      singleColor,    1+3(destAddress)
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

        sq      outputStq3,     0+6(destAddress)
        sq      singleColor,    1+6(destAddress)
        sq.xyz  vertex3,        2+6(destAddress)
        iaddiu  vertexDataFrom, vertexDataFrom, 3
        iaddiu  stqDataFrom,    stqDataFrom,    3
        iaddiu  destAddress,    destAddress,    9
        iaddi   vertexCounter,  vertexCounter,  -3  
        ibne    vertexCounter,  buffer, vertexLoop  
    xgkick kickAddress 
--barrier 
--cont
    b   begin
--exit
--endexit


.name DynPipVU1C
.syntax new
.name DynPipVU1_C
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


   loi         255
   mini.xyz    singleColor, singleColor, i
   max.xyz     singleColor, singleColor, vf00[x]
   ftoi0       singleColor, singleColor

begin:
    xtop buffer

   lq      primTag,   1(buffer)

    ilw.w   vertexCount,        0(buffer)
    iaddiu  vertexDataFrom,     buffer,         2
    iadd    kickAddress,        vertexDataFrom, vertexCount
    iadd    destAddress,        kickAddress,    vertexCount
    iadd    kickAddress,        kickAddress,    vertexCount

   sq gifSetTag,            0(destAddress)
   sq testsTag,             1(destAddress)
   sq gifSetTag,            2(destAddress)
   sq lodGifTag,            3(destAddress)
   sq primTag,              4(destAddress)
   iaddiu                     destAddress,    destAddress,    5


   lq.y  interp, 8(vi00)


   lq.xyz  scale,  0(buffer)

    iadd vertexCounter, buffer, vertexCount
vertexLoop:
        iadd    vertexDataTo,   vertexDataFrom, vertexCount
        lq      vertex1From,    (vertexDataFrom)
        lq      vertex1To,      (vertexDataTo)

   sub   temp1,      vertex1To,    vertex1From
   mul   temp2,      temp1,   interp[y]
   add   vertex1,   temp2,   vertex1From


   mul            acc,           mvp[0], vertex1[x]
   madd           acc,           mvp[1], vertex1[y]
   madd           acc,           mvp[2], vertex1[z]
   madd           vertex1, mvp[3], vertex1[w]


   clipw.xyz	vertex1,   vertex1	
   fcand       VI01,       0x3FFFF
   iaddiu      adcBit,     VI01, 0x7FFF
   isw.w       adcBit,     1(destAddress)


   div            q, vf00[w], vertex1[w]
   mul.xyz        vertex1, vertex1, q


   mula.xyz    acc,        scale,    vf00[w]
   madd.xyz    vertex1,   vertex1,   scale
   ftoi4.xyz   vertex1,   vertex1

        sq      singleColor,    0(destAddress)
        sq.xyz  vertex1,        1(destAddress)
        lq      vertex2From,    1(vertexDataFrom)
        lq      vertex2To,      1(vertexDataTo)

   sub   temp1,      vertex2To,    vertex2From
   mul   temp2,      temp1,   interp[y]
   add   vertex2,   temp2,   vertex2From


   mul            acc,           mvp[0], vertex2[x]
   madd           acc,           mvp[1], vertex2[y]
   madd           acc,           mvp[2], vertex2[z]
   madd           vertex2, mvp[3], vertex2[w]


   clipw.xyz	vertex2,   vertex2	
   fcand       VI01,       0x3FFFF
   iaddiu      adcBit,     VI01, 0x7FFF
   isw.w       adcBit,     1+2(destAddress)


   div            q, vf00[w], vertex2[w]
   mul.xyz        vertex2, vertex2, q


   mula.xyz    acc,        scale,    vf00[w]
   madd.xyz    vertex2,   vertex2,   scale
   ftoi4.xyz   vertex2,   vertex2

        sq      singleColor,    0+2(destAddress)
        sq.xyz  vertex2,        1+2(destAddress)
        lq      vertex3From,    2(vertexDataFrom)
        lq      vertex3To,      2(vertexDataTo)

   sub   temp1,      vertex3To,    vertex3From
   mul   temp2,      temp1,   interp[y]
   add   vertex3,   temp2,   vertex3From


   mul            acc,           mvp[0], vertex3[x]
   madd           acc,           mvp[1], vertex3[y]
   madd           acc,           mvp[2], vertex3[z]
   madd           vertex3, mvp[3], vertex3[w]


   clipw.xyz	vertex3,   vertex3	
   fcand       VI01,       0x3FFFF
   iaddiu      adcBit,     VI01, 0x7FFF
   isw.w       adcBit,     1+4(destAddress)


   div            q, vf00[w], vertex3[w]
   mul.xyz        vertex3, vertex3, q


   mula.xyz    acc,        scale,    vf00[w]
   madd.xyz    vertex3,   vertex3,   scale
   ftoi4.xyz   vertex3,   vertex3

        sq      singleColor,    0+4(destAddress)
        sq.xyz  vertex3,        1+4(destAddress)
        iaddiu  vertexDataFrom, vertexDataFrom, 3
        iaddiu  destAddress,    destAddress,    6
        iaddi   vertexCounter,  vertexCounter,  -3  
        ibne    vertexCounter,  buffer, vertexLoop  
    xgkick kickAddress 
--barrier 
--cont
    b   begin
--exit
--endexit

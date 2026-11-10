
.name StaPipVU1CullTC
.syntax new
.name StaPipVU1Cull_TC
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

begin:
    xtop buffer

   lq.xyz  scale,                  0(buffer)
   lq      primTag,                1(buffer)

    iaddiu  vertexData,         buffer,         2
    ilw.w   vertexCount,        0(buffer)
    iadd    stqData,            vertexData,     vertexCount
    iadd    colorData,          stqData,        vertexCount
    iblez   singleColorEnabled, setDestAddrMultiColor
    iadd    kickAddress,        stqData,        vertexCount
    b       setDestAddr
setDestAddrMultiColor:
    iadd    kickAddress,        colorData,      vertexCount
setDestAddr:
    iaddiu  destAddress,        kickAddress,    0

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
        iblez  singleColorEnabled, multiColor
        add      color1,    vf00,   singleColor
        add      color2,    vf00,   singleColor
        add      color3,    vf00,   singleColor
        b processing
multiColor:
        lq      color1,   (colorData)
        lq      color2,   1(colorData)
        lq      color3,   2(colorData)
processing:
        lq      vertex1,  (vertexData)
        lq      stq1,     (stqData)
        lq      vertex2,  1(vertexData)
        lq      stq2,     1(stqData)
        lq      vertex3,  2(vertexData)
        lq      stq3,     2(stqData)

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


   loi         255
   mini.xyz    color1, color1, i
   max.xyz     color1, color1, vf00[x]
   ftoi0       color1, color1


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


   loi         255
   mini.xyz    color2, color2, i
   max.xyz     color2, color2, vf00[x]
   ftoi0       color2, color2


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


   loi         255
   mini.xyz    color3, color3, i
   max.xyz     color3, color3, vf00[x]
   ftoi0       color3, color3

        sq      outputStq1,     0(destAddress)
        sq      color1,         1(destAddress)
        sq.xyz  vertex1,        2(destAddress)
        sq      outputStq2,     0+3(destAddress)
        sq      color2,         1+3(destAddress)
        sq.xyz  vertex2,        2+3(destAddress)
        sq      outputStq3,     0+6(destAddress)
        sq      color3,         1+6(destAddress)
        sq.xyz  vertex3,        2+6(destAddress)
        iaddiu  vertexData,     vertexData,     3                         
        iaddiu  stqData,        stqData,        3  
        iaddiu  colorData,      colorData,      3  
        iaddiu  destAddress,    destAddress,    9
        iaddi   vertexCounter,  vertexCounter,  -3  
        ibne    vertexCounter,  buffer, vertexLoop  
    xgkick kickAddress 
--barrier 
--cont
    b   begin
--exit
--endexit

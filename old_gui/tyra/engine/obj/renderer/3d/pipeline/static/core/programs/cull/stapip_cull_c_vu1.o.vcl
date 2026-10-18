
.name StaPipVU1CullC
.syntax new
.name StaPipVU1Cull_C
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

begin:
    xtop buffer

   lq.xyz  scale,                  0(buffer)
   lq      primTag,                1(buffer)

    iaddiu  vertexData,         buffer,         2
    ilw.w   vertexCount,        0(buffer)
    iadd    colorData,          vertexData,     vertexCount
    iblez   singleColorEnabled, setDestAddrMultiColor
    iadd    kickAddress,        vertexData,     vertexCount
    b       setDestAddr
setDestAddrMultiColor:
    iadd    kickAddress,        colorData,      vertexCount
setDestAddr:
    iaddiu  destAddress,        kickAddress,    0

   sq gifSetTag,            0(destAddress)
   sq testsTag,             1(destAddress)
   sq gifSetTag,            2(destAddress)
   sq lodGifTag,            3(destAddress)
   sq primTag,              4(destAddress)
   iaddiu                     destAddress,    destAddress,    5

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
        lq      vertex2,  1(vertexData)
        lq      vertex3,  2(vertexData)

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
   isw.w       adcBit,     1+2(destAddress)


   div            q, vf00[w], vertex2[w]
   mul.xyz        vertex2, vertex2, q


   mula.xyz    acc,        scale,    vf00[w]
   madd.xyz    vertex2,   vertex2,   scale
   ftoi4.xyz   vertex2,   vertex2


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
   isw.w       adcBit,     1+4(destAddress)


   div            q, vf00[w], vertex3[w]
   mul.xyz        vertex3, vertex3, q


   mula.xyz    acc,        scale,    vf00[w]
   madd.xyz    vertex3,   vertex3,   scale
   ftoi4.xyz   vertex3,   vertex3


   loi         255
   mini.xyz    color3, color3, i
   max.xyz     color3, color3, vf00[x]
   ftoi0       color3, color3

        sq      color1,         0(destAddress)
        sq.xyz  vertex1,        1(destAddress)
        sq      color2,         0+2(destAddress)
        sq.xyz  vertex2,        1+2(destAddress)
        sq      color3,         0+4(destAddress)
        sq.xyz  vertex3,        1+4(destAddress)
        iaddiu  vertexData,     vertexData,     3      
        iaddiu  colorData,      colorData,      3  
        iaddiu  destAddress,    destAddress,    6
        iaddi   vertexCounter,  vertexCounter,  -3  
        ibne    vertexCounter,  buffer, vertexLoop  
    xgkick kickAddress 
--barrier 
--cont
    b   begin
--exit
--endexit

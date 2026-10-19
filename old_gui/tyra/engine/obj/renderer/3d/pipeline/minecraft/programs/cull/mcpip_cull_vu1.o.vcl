
.name VU1BlocksCull
.syntax new
.name VU1BlocksCull
.vu
.init_vf_all
.init_vi_all
--enter
--endenter

   iaddiu  staticVertexData,  vi00, 4
   iaddiu  staticStqData,    vi00, 4 + 36


   fcset   0x000000

begin:
xtop        buffer
iaddiu      blockData,  buffer, 6

   lq.xyz   scale,          0(buffer)
   ilw.w    blocksCount,    0(buffer)
   lq       clutTex,        1(buffer)
   lq       viewProj[0],    0+2(buffer)
   lq       viewProj[1],    1+2(buffer)
   lq       viewProj[2],    2+2(buffer)
   lq       viewProj[3],    3+2(buffer)

iadd    blockCounter,   buffer, blocksCount
blocksLoop:

   lq    model[0],    0(blockData)
   lq    model[1],    1(blockData)
   lq    model[2],    2(blockData)
   lq    model[3],    3(blockData)
   lq    color,       4(blockData)
   lq    stOffset,    5(blockData)


   mul            acc,           viewProj[0], model[0][x]
   madd           acc,           viewProj[1], model[0][y]
   madd           acc,           viewProj[2], model[0][z]
   madd           mvp[0],  viewProj[3], model[0][w]
   mul            acc,           viewProj[0], model[1][x]
   madd           acc,           viewProj[1], model[1][y]
   madd           acc,           viewProj[2], model[1][z]
   madd           mvp[1],  viewProj[3], model[1][w]
   mul            acc,           viewProj[0], model[2][x]
   madd           acc,           viewProj[1], model[2][y]
   madd           acc,           viewProj[2], model[2][z]
   madd           mvp[2],  viewProj[3], model[2][w]
   mul            acc,           viewProj[0], model[3][x]
   madd           acc,           viewProj[1], model[3][y]
   madd           acc,           viewProj[2], model[3][z]
   madd           mvp[3],  viewProj[3], model[3][w]


   loi         255
   mini.xyz    color, color, i
   max.xyz     color, color, vf00[x]
   ftoi0       color, color


   ilw.w    static1,             3(vi00)
   ilw.x    currDBufferOffset,   3(vi00)
   ibgtz    currDBufferOffset,   get_dest_addr_second
   get_dest_addr_first:
   iaddiu   destAddress,          vi00,    774
   isw.x    static1,             3(vi00)
   b get_dest_addr_finish
   get_dest_addr_second:
   iaddiu   destAddress,          vi00,    887
   isw.x    vi00,                3(vi00)
   get_dest_addr_finish:
   iaddiu   kickAddress,          destAddress,    0


   lq       setTag,     2(vi00)
   lq       lodTag,     0(vi00)
   sq       setTag,     0(destAddress)
   sq       lodTag,     1(destAddress)
   sq       setTag,     2(destAddress)
   sq       clutTex,  3(destAddress)
   lq       primTag,    1(vi00)
   sq       primTag,    4(destAddress)
   iaddiu   destAddress, destAddress,    5

    iaddiu      vertexData,     staticVertexData,   0
    iaddiu      stqData,        staticStqData,      0
    iaddiu vertexCounter, buffer, 36
vertexLoop:

   lq    vertex1,   0(vertexData)
   lq    stq1,      0(stqData)


   lq    vertex2,   1(vertexData)
   lq    stq2,      1(stqData)


   lq    vertex3,   2(vertexData)
   lq    stq3,      2(stqData)


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


   add    stq1,   stq1, stOffset


   mulq  outputStq1,  stq1,  q


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


   add    stq2,   stq2, stOffset


   mulq  outputStq2,  stq2,  q


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


   add    stq3,   stq3, stOffset


   mulq  outputStq3,  stq3,  q

        sq      outputStq1,     0(destAddress)
        sq      color,          1(destAddress)
        sq.xyz  vertex1,        2(destAddress)
        sq      outputStq2,     0+3(destAddress)
        sq      color,          1+3(destAddress)
        sq.xyz  vertex2,        2+3(destAddress)
        sq      outputStq3,     0+6(destAddress)
        sq      color,          1+6(destAddress)
        sq.xyz  vertex3,        2+6(destAddress)
        iaddiu  vertexData,     vertexData,     3                         
        iaddiu  stqData,        stqData,        3  
        iaddiu  destAddress,    destAddress,    9
        iaddi   vertexCounter,  vertexCounter,  -3
        ibne    vertexCounter,  buffer,         vertexLoop
--barrier
    xgkick  kickAddress
    iaddiu  blockData,      blockData,      6  
    iaddi   blockCounter,   blockCounter,   -1
    ibne    blockCounter,   buffer,         blocksLoop
--cont
b begin
--exit
--endexit


.name VU1BlocksAsIs
.syntax new
.name VU1BlocksAsIs
.vu
.init_vf_all
.init_vi_all
--enter
--endenter

   lq       lodTag,            0(vi00)
   lq       setTag,            1(vi00)

begin:
    xtop buffer

   lq       scale,          0(buffer)
   lq       primTag,           1(buffer)
   lq       clut,           2(buffer)
   lq       color,          3(buffer)
   ilw.w    vertexCount,    0(buffer)

    iaddiu  vertexData,         buffer,         4
    iadd    stqData,            vertexData,     vertexCount
    iadd    destAddress,        stqData,        vertexCount
    iadd    kickAddress,        stqData,        vertexCount

   sq setTag,         0(destAddress)
   sq lodTag,         1(destAddress)
   sq setTag,         2(destAddress)
   sq clut,           3(destAddress)
   sq primTag,        4(destAddress)
   iaddiu               destAddress,    destAddress,    5


   loi         255
   mini.xyz    color, color, i
   max.xyz     color, color, vf00[x]
   ftoi0       color, color

    iadd vertexCounter, buffer, vertexCount
vertexLoop:
        lq      vertex1,  (vertexData)
        lq      stq1,     (stqData)
        lq      vertex2,  1(vertexData)
        lq      stq2,     1(stqData)
        lq      vertex3,  2(vertexData)
        lq      stq3,     2(stqData)

   mula.xyz    acc,        scale,    vf00[w]
   madd.xyz    vertex1,   vertex1,   scale
   ftoi4.xyz   vertex1,   vertex1

        div q,  vf00[w],    vertex1[w]

   mulq  outputStq1,  stq1,  q


   mula.xyz    acc,        scale,    vf00[w]
   madd.xyz    vertex2,   vertex2,   scale
   ftoi4.xyz   vertex2,   vertex2

        div q,  vf00[w],    vertex2[w]

   mulq  outputStq2,  stq2,  q


   mula.xyz    acc,        scale,    vf00[w]
   madd.xyz    vertex3,   vertex3,   scale
   ftoi4.xyz   vertex3,   vertex3

        div q,  vf00[w],    vertex3[w]

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
        ibne    vertexCounter,  buffer, vertexLoop  
    xgkick kickAddress 
--barrier 
--cont
    b   begin
--exit
--endexit

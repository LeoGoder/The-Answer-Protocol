
.name VU1DrawFinish
.syntax new
.name VU1DrawFinish
.vu
.init_vf_all
.init_vi_all
--enter
--endenter
    xtop    buffer
    iaddiu  kickAddress,    buffer, 10
    xgkick  kickAddress
--exit
--endexit

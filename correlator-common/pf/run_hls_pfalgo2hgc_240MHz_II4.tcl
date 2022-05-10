# get the configuration
set pfBoard "VCU118"
set pfReg "HGCal"
set hlsIPVersion 25.4

set cflags "-std=c++0x -DREG_${pfReg} -DBOARD_${pfBoard} -DHLS_pipeline_II=4 -DL1PF_DSP_LATENCY3"

open_project -reset "proj_pf${pfReg}_${pfBoard}_240MHz_II4"
if { $pfBoard == "none" } {
    set hlsTopFunc pfalgo2hgc
} else {
    set hlsTopFunc packed_pfalgo2hgc
}
set_top ${hlsTopFunc}
add_files firmware/pfalgo2hgc.cpp -cflags "${cflags}"
add_files -tb pfalgo2hgc_test.cpp  -cflags "${cflags}"
add_files -tb ref/pfalgo2hgc_ref.cpp  -cflags "${cflags}"
add_files -tb ref/pfalgo_common_ref.cpp  -cflags "${cflags}"
add_files -tb ../dataformats/layer1_emulator.cpp -cflags "${cflags}"
add_files -tb ../utils/pattern_serializer.cpp -cflags "${cflags}"
add_files -tb ../utils/test_utils.cpp -cflags "${cflags}"
add_files -tb ../data/TTbar_PU200_HGCal.dump

# reset the solution
open_solution -reset "solution"
set_part {xcvu9p-flga2104-2L-e}
create_clock -period 2.5 -name default

config_interface -trim_dangling_port

csim_design

csynth_design

exit

if { [ info exists env(pfBoard) ] } { set pfBoard $env(pfBoard) } { set pfBoard "none" }
if { [ info exists env(pfReg) ] } { set pfReg $env(pfReg) } { set pfReg "HGCal" }
set cflags "-std=c++0x -DREG_${pfReg} -DBOARD_${pfBoard}"

open_project -reset "proj_pfeginput-${pfReg}_${pfBoard}"

if { $pfBoard == "none" } {
    set_top select_eginput
} else {
    set_top packed_select_eginput
}
add_files firmware/pfeginput.cpp -cflags "${cflags}"
add_files -tb pfeginput_test.cpp  -cflags "${cflags}"
add_files -tb ref/pfeginput_ref.cpp  -cflags "${cflags}"
add_files -tb ../../dataformats/layer1_emulator.cpp -cflags "${cflags}"

add_files -tb ../../utils/pattern_serializer.cpp -cflags "${cflags}"
add_files -tb ../../utils/test_utils.cpp -cflags "${cflags}"
add_files -tb ../../data/TTbar_PU200_HGCal.dump

# reset the solution
open_solution -reset "solution"
set_part {xcvu9p-flga2104-2L-e}
create_clock -period 2.2 -name default

config_interface -trim_dangling_port
config_rtl -reset none  
# do stuff
csim_design
if { [info exists env(DO_SYNTH)] && $env(DO_SYNTH) == "1"  } {
    csynth_design
    if { [info exists env(DO_COSIM)] && $env(DO_COSIM) == "1"  } {
        cosim_design -trace_level all
    }
}

# exit Vivado HLS
exit

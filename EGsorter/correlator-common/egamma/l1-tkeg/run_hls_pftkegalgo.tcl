# get the configuration
if { [ info exists env(pfBoard) ] } { set pfBoard $env(pfBoard) } { set pfBoard "none" }
#set pfBoard "VCU118"
if { [ info exists env(pfReg) ] } { set pfReg $env(pfReg) } { set pfReg "HGCal" }

if { $pfReg == "HGCal" } {
  set pipelineII 4
} else {
  set pipelineII 2
}

set cflags "-std=c++0x -DREG_${pfReg} -DBOARD_${pfBoard} -DHLS_pipeline_II=${pipelineII}"

# open the project, don't forget to reset
open_project -reset "proj_tkeg${pfReg}_${pfBoard}"


if { $pfBoard == "none" } {
    set_top pftkegalgo
} else {
    set_top packed_pftkegalgo
}
add_files firmware/pftkegalgo.cpp -cflags "${cflags}"
add_files -tb pftkegalgo_test.cpp  -cflags "${cflags}"
add_files -tb ref/pftkegalgo_ref.cpp  -cflags "${cflags}"
add_files -tb ../../dataformats/layer1_emulator.cpp -cflags "${cflags}"

# add_files -tb ref/pfalgo_common_ref.cpp  -cflags "${cflags}"
add_files -tb ../../utils/pattern_serializer.cpp -cflags "${cflags}"
add_files -tb ../../utils/test_utils.cpp -cflags "${cflags}"
add_files -tb ../../data/TTbar_PU200_${pfReg}.dump
# add_files -tb ../../data/DoubleElectron_PU200_${pfReg}.dump

# reset the solution
open_solution -reset "solution"
set_part {xcvu9p-flga2104-2L-e}
create_clock -period 2.5 -name default

config_interface -trim_dangling_port
# do stuff
csim_design
if { [info exists env(DO_SYNTH)] && $env(DO_SYNTH) == "1"  } {
    csynth_design
    if { [info exists env(DO_COSIM)] && $env(DO_COSIM) == "1"  } {
        cosim_design -trace_level all
    }
    if { [info exists env(DO_EXPORT)] && $env(DO_EXPORT) == "1"  } {
        export_design -format ip_catalog -vendor "cern-cms" -version ${hlsIPVersion} -description "${hlsTopFunc}"
    }
}

# exit Vivado HLS
exit

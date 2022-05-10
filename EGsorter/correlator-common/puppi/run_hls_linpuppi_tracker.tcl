if { [ info exists env(puppiReg) ] } { set puppiReg $env(puppiReg) } { set puppiReg "Barrel" }
if { [ info exists env(puppiBoard) ] } { set puppiBoard $env(puppiBoard) } { set puppiBoard "none" }
set cflags "-std=c++0x -DREG_${puppiReg} -DBOARD_${puppiBoard}" 

open_project -reset proj_linpuppi_${puppiReg}_${puppiBoard}

if { $puppiBoard == "none" } {
    set_top linpuppiNoCrop
    #set_top linpuppi
    #set_top linpuppi_chs
} else {
    set_top packed_linpuppiNoCrop
    #set_top packed_linpuppi
    #set_top packed_linpuppi_chs
}
add_files firmware/linpuppi.cpp  -cflags "${cflags}"
add_files -tb linpuppi_ref.cpp   -cflags "${cflags}"
add_files -tb ../utils/test_utils.cpp  -cflags "${cflags}"
add_files -tb ../utils/pattern_serializer.cpp -cflags "${cflags}"
add_files -tb ../dataformats/layer1_emulator.cpp -cflags "${cflags}"
add_files -tb linpuppi_test.cpp   -cflags "${cflags} -DTEST_PUPPI_NOCROP -DTEST_PT_CUT=80" 
#add_files -tb linpuppi_test.cpp   -cflags "${cflags} -DTEST_PT_CUT=80"
if { $puppiReg == "Barrel" } {
    add_files -tb ../pf/ref/pfalgo3_ref.cpp   -cflags "${cflags}"
} elseif { $puppiReg == "HGCal" } {
    add_files -tb ../pf/ref/pfalgo2hgc_ref.cpp   -cflags "${cflags}"
}
add_files -tb ../pf/ref/pfalgo_common_ref.cpp   -cflags "${cflags}"
add_files -tb ../data/TTbar_PU200_${puppiReg}.dump

# reset the solution
open_solution -reset "solution"
set_part {xcvu9p-flga2104-2L-e}
create_clock -period 3.0 -name default

config_interface -trim_dangling_port
# do stuff
csim_design
if { [info exists env(DO_SYNTH)] && $env(DO_SYNTH) == "1"  } {
    csynth_design
    if { [info exists env(DO_COSIM)] && $env(DO_COSIM) == "1"  } {
        cosim_design -trace_level all
    }
    if { [info exists env(DO_EXPORT)] && $env(DO_EXPORT) == "1"  } {
        export_design -format ip_catalog -vendor "cern-cms" 
    }
}

# exit Vivado HLS
exit

# Configuration
if { [ info exists env(pfBoard) ] } { set pfBoard $env(pfBoard) } { set pfBoard "none" }
#set pfBoard "VCU118"
if { [ info exists env(pfReg) ] } { set pfReg $env(pfReg) } { set pfReg "HGCal" }
    
set cflags "-std=c++0x -DREG_${pfReg} -DBOARD_${pfBoard}"

if { $pfBoard == "none" } {
    set funcs { "packed_pftkegsorter_barrel_pho" "packed_pftkegsorter_barrel_ele" }
} else {
    set funcs { "packed_pftkegsorter_barrel_pho" "packed_pftkegsorter_barrel_ele" }
}

foreach func ${funcs} {
    # Project
    open_project -reset "proj_${func}_${pfReg}"
    set_top ${func}

    add_files firmware/pftkegsorter_barrel.cpp -cflags "${cflags}"
    add_files -tb pftkegsorter_barrel_test.cpp -cflags "${cflags}"
    add_files -tb ../../dataformats/layer1_emulator.cpp -cflags "${cflags}"
    add_files -tb ../../utils/test_utils.cpp -cflags "${cflags}"
    add_files -tb ../../data/TTbar_PU200_${pfReg}.dump

    # Solution
    open_solution -reset "solution"
    set_part {xcvu9p-flga2104-2L-e}
    create_clock -period 2.7 -name default

    # Sim, synth, etc.
    csim_design -argv TTbar_PU200,${pfReg}
    if { [info exists env(DO_SYNTH)] && $env(DO_SYNTH) == "1"  } {
        csynth_design
    }
}

exit

source configIP.tcl

#set cflags "-std=c++0x -DREG_${pfReg} -DBOARD_${pfBoard} ${regionizerCFlags} -DTKROUTER_NOVTX"
set cflags "-std=c++0x -DREG_${pfReg} -DBOARD_${pfBoard} ${regionizerCFlags}"
open_project -reset "project_csim_${pfReg}_${regionizerMode}"

set_top ${hlsTopFunc}

add_files firmware/calo_regionizer.cpp -cflags "${cflags}"
add_files firmware/tk_regionizer.cpp -cflags "${cflags}"
add_files firmware/mu_regionizer.cpp -cflags "${cflags}"
add_files -tb multififo_regionizer_ref.cpp -cflags "${cflags}"
add_files -tb regionizer_test.cpp -cflags "${cflags}"
add_files -tb ../common/regionizer_base_ref.cpp -cflags "${cflags}"
add_files -tb ../../egamma/l1-input/ref/pfeginput_ref.cpp -cflags "${cflags}"
add_files -tb ../../dataformats/layer1_emulator.cpp -cflags "${cflags}"
add_files -tb ../../utils/pattern_serializer.cpp -cflags "${cflags}"
add_files -tb ../../utils/test_utils.cpp -cflags "${cflags}"
add_files -tb ../../data/TTbar_PU200_${pfReg}.dump

open_solution -reset "solution"
set_part {xcvu9p-flga2104-2L-e}
create_clock -period 2.5

csim_design
exit

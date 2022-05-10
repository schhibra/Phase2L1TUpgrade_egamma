#ifndef FIRMWARE_PFEGINPUT_H
#define FIRMWARE_PFEGINPUT_H

#include "../../../dataformats/layer1_objs.h"
#include "../../../dataformats/layer1_multiplicities.h"

#define EGINPUT_BITMASK 0x1E

void packed_select_eginput(const ap_uint<l1ct::HadCaloObj::BITWIDTH> in,
                           bool valid_in,
                           ap_uint<l1ct::EmCaloObj::BITWIDTH> &out,
                           bool &valid_out);

void select_eginput(const l1ct::HadCaloObj &in, bool valid_in, l1ct::EmCaloObj & out, bool & valid_out);

#endif

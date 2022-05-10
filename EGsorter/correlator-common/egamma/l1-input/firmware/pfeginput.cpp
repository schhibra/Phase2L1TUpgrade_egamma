#include "pfeginput.h"

using namespace l1ct;

void select_eginput(const HadCaloObj &in, bool valid_in, EmCaloObj & out, bool & valid_out) {
  const emid_t MASK = emid_t(EGINPUT_BITMASK);
  valid_out = valid_in && (in.hwEmID & MASK).or_reduce();
  out.hwPt = in.hwEmPt;
  out.hwEta = in.hwEta;
  out.hwPhi = in.hwPhi;
  out.hwPtErr = 0;
  out.hwEmID = in.hwEmID;
}


void packed_select_eginput(const ap_uint<l1ct::HadCaloObj::BITWIDTH> in,
                           bool valid_in,
                           ap_uint<l1ct::EmCaloObj::BITWIDTH> &out,
                           bool &valid_out) {
  #pragma HLS latency min=1
  #pragma HLS pipeline II=1
  #pragma HLS interface port=out ap_none
  #pragma HLS interface port=valid_out ap_none

  EmCaloObj out_em;
  select_eginput(HadCaloObj::unpack(in), valid_in, out_em, valid_out);
  out = out_em.pack();
}

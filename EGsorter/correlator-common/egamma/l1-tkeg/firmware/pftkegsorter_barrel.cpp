#include "pftkegsorter_barrel.h"

void packed_pftkegsorter_barrel_pho(bool newBoard, bool lastregion,
				    const ap_uint<l1ct::PFRegion::BITWIDTH> & packed_region,
				    const ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_objs_in[NOBJ],
				    ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_objs_out[NL1_EGOUT],
				    unsigned int Nboards, bool &validout){
  #pragma HLS pipeline II=3

  #pragma HLS ARRAY_PARTITION variable=packed_objs_in complete
  #pragma HLS ARRAY_PARTITION variable=packed_objs_out complete

  l1ct::PFRegion region;
  l1ct::EGIsoObj objs_in[NOBJ]; 
  l1ct::EGIsoObj objs_out[NL1_EGOUT];
  
  #pragma HLS ARRAY_PARTITION variable=objs_in complete
  #pragma HLS ARRAY_PARTITION variable=objs_out complete
  
  pftkegsorter_barrel_unpack_in(packed_region, packed_objs_in, region, objs_in);
  pftkegsorter_barrel(newBoard, lastregion, region, objs_in, objs_out, Nboards, validout);
  pftkegsorter_barrel_pack_out(objs_out, packed_objs_out);                         
}

void packed_pftkegsorter_barrel_ele(bool newBoard, bool lastregion,
				    const ap_uint<l1ct::PFRegion::BITWIDTH> & packed_region,
				    const ap_uint<l1ct::EGIsoEleObj::BITWIDTH> packed_objs_in[NOBJ],
				    ap_uint<l1ct::EGIsoEleObj::BITWIDTH> packed_objs_out[NL1_EGOUT],
				    unsigned int Nboards, bool &validout){
  #pragma HLS pipeline II=3

  #pragma HLS ARRAY_PARTITION variable=packed_objs_in complete
  #pragma HLS ARRAY_PARTITION variable=packed_objs_out complete

  l1ct::PFRegion region;
  l1ct::EGIsoEleObj objs_in[NOBJ]; 
  l1ct::EGIsoEleObj objs_out[NL1_EGOUT];
  
  #pragma HLS ARRAY_PARTITION variable=objs_in complete
  #pragma HLS ARRAY_PARTITION variable=objs_out complete
  
  pftkegsorter_barrel_unpack_in(packed_region, packed_objs_in, region, objs_in);
  pftkegsorter_barrel(newBoard, lastregion, region, objs_in, objs_out, Nboards, validout);
  pftkegsorter_barrel_pack_out(objs_out, packed_objs_out);                         
}

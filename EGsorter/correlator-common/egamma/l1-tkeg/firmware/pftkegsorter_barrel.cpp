#include "pftkegsorter_barrel.h"

void packed_pftkegsorter_barrel_pho(bool newEvent, bool lastregion,
				    const ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_objs_in[NOBJ],
				    ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_objs_out[NL1_EGOUT]){
  #pragma HLS pipeline II=3

  #pragma HLS ARRAY_PARTITION variable=packed_objs_in complete
  #pragma HLS ARRAY_PARTITION variable=packed_objs_out complete

  l1ct::EGIsoObj objs_in[NOBJ]; 
  l1ct::EGIsoObj objs_out[NL1_EGOUT];
  
  #pragma HLS ARRAY_PARTITION variable=objs_in complete
  #pragma HLS ARRAY_PARTITION variable=objs_out complete
  
  pftkegsorter_barrel_unpack_in<l1ct::EGIsoObj>(packed_objs_in, objs_in);
  pftkegsorter_barrel<l1ct::EGIsoObj>(newEvent, lastregion, objs_in, objs_out);
  pftkegsorter_barrel_pack_out<l1ct::EGIsoObj>(objs_out, packed_objs_out);                         
}

void packed_pftkegsorter_barrel_ele(bool newEvent, bool lastregion,
				    const ap_uint<l1ct::EGIsoEleObj::BITWIDTH> packed_objs_in[NOBJ],
				    ap_uint<l1ct::EGIsoEleObj::BITWIDTH> packed_objs_out[NL1_EGOUT]){
  #pragma HLS pipeline II=3

  #pragma HLS ARRAY_PARTITION variable=packed_objs_in complete
  #pragma HLS ARRAY_PARTITION variable=packed_objs_out complete

  l1ct::EGIsoEleObj objs_in[NOBJ]; 
  l1ct::EGIsoEleObj objs_out[NL1_EGOUT];
  
  #pragma HLS ARRAY_PARTITION variable=objs_in complete
  #pragma HLS ARRAY_PARTITION variable=objs_out complete
  
  pftkegsorter_barrel_unpack_in<l1ct::EGIsoEleObj>(packed_objs_in, objs_in);
  pftkegsorter_barrel<l1ct::EGIsoEleObj>(newEvent, lastregion, objs_in, objs_out);
  pftkegsorter_barrel_pack_out<l1ct::EGIsoEleObj>(objs_out, packed_objs_out);                         
}

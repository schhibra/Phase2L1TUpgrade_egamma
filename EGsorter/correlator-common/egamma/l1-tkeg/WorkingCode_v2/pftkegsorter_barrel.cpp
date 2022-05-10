#include "pftkegsorter_barrel.h"

void packed_pftkegsorter_barrel_pho(bool newEvent, bool lastregion, bool region_odd,
			   const ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_objs_in[NOBJ],
			   ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_objs_out[NL1_EGOUT]){
  #pragma HLS pipeline II=3

  #pragma HLS ARRAY_PARTITION variable=packed_objs_in complete
  #pragma HLS ARRAY_PARTITION variable=packed_objs_out complete
  
  
  l1ct::EGIsoObj objs_in[NOBJ]; 
  l1ct::EGIsoObj objs_out[NL1_EGOUT];
  
  #pragma HLS ARRAY_PARTITION variable=objs_in complete
  #pragma HLS ARRAY_PARTITION variable=objs_out complete
  
  pftkegsorter_barrel_unpack_in(packed_objs_in, objs_in);
  pftkegsorter_barrel(newEvent, lastregion, region_odd, objs_in, objs_out);
  pftkegsorter_barrel_pack_out(objs_out, packed_objs_out);                         
}

void packed_pftkegsorter_barrel_ele(bool newEvent, bool lastregion, bool region_odd,
		       const ap_uint<l1ct::EGIsoEleObj::BITWIDTH> packed_objs_in[NOBJ],
                       ap_uint<l1ct::EGIsoEleObj::BITWIDTH> packed_objs_out[NL1_EGOUT]){
  #pragma HLS pipeline II=3

  #pragma HLS ARRAY_PARTITION variable=packed_objs_in complete
  #pragma HLS ARRAY_PARTITION variable=packed_objs_out complete

  l1ct::EGIsoEleObj objs_in[NOBJ]; 
  l1ct::EGIsoEleObj objs_out[NL1_EGOUT];
  
  #pragma HLS ARRAY_PARTITION variable=objs_in complete
  #pragma HLS ARRAY_PARTITION variable=objs_out complete
  
  pftkegsorter_barrel_unpack_in(packed_objs_in, objs_in);
  pftkegsorter_barrel(newEvent, lastregion, region_odd, objs_in, objs_out);
  pftkegsorter_barrel_pack_out(objs_out, packed_objs_out);                         
}

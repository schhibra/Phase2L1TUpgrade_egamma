#ifndef FIRMWARE_PFTKEGSORTER_BARREL_H
#define FIRMWARE_PFTKEGSORTER_BARREL_H

#include "../../../dataformats/layer1_multiplicities.h"
#include "../../../dataformats/layer1_emulator.h"
#include "../../../common/firmware/bitonic_hybrid.h"

#define NOBJ 10
#define NL1_EGOUT 16

// BITONIC SORTER (CALLED FROM MAIN SORTER) ////////////////////////////////////////
template<typename T>
void get_bitonic_sequence(const T in_board1[NOBJ], const T in_board2[NOBJ], T bitonic_in[2*NOBJ]) {//10, 10, 20
  
 #pragma HLS inline
 #pragma HLS array_partition variable=in_board1 complete
 #pragma HLS array_partition variable=in_board2 complete
 #pragma HLS array_partition variable=bitonic_in complete
  
 make_bitonic_loop: for(int id = 0, ia=NOBJ-1; id<NOBJ; id++, ia--) {
    #pragma HLS UNROLL
    bitonic_in[id] = in_board1[ia];
    bitonic_in[id+NOBJ] = in_board2[id];
  }
}
template<typename T>
void get_bitonic_sequence_1(const T in_board1[NL1_EGOUT], const T in_board2[NL1_EGOUT], T bitonic_in[2*NL1_EGOUT]) {//16, 16, 32
  
 #pragma HLS inline
 #pragma HLS array_partition variable=in_board1 complete
 #pragma HLS array_partition variable=in_board2 complete
 #pragma HLS array_partition variable=bitonic_in complete
  
 make_bitonic_loop: for(int id = 0, ia=NL1_EGOUT-1; id<NL1_EGOUT; id++, ia--) {
    #pragma HLS UNROLL
    bitonic_in[id] = in_board1[ia];
    bitonic_in[id+NL1_EGOUT] = in_board2[id];
  }
}

template<typename T>
void merge_sort(const T in1[NOBJ], const T in2[NOBJ], T sorted_out[NL1_EGOUT]) {//10, 10, 16
  
  #pragma HLS pipeline II=6
  //#pragma HLS inline
  #pragma HLS array_partition variable=in1 complete
  #pragma HLS array_partition variable=in2 complete
  #pragma HLS array_partition variable=sorted_out complete
  
  T bitonic_in[2*NOBJ];
  get_bitonic_sequence(in1, in2, bitonic_in);
  hybridBitonicSort::bitonicMerger<T, 2*NOBJ, 0>::run(bitonic_in, 0);
  
  for(int i = 0; i < NL1_EGOUT; i++) {
    sorted_out[i] = bitonic_in[i];
  }
}
template<typename T>
void merge_sort_1(const T in1[NL1_EGOUT], const T in2[NL1_EGOUT], T sorted_out[NL1_EGOUT]) {//16, 16, 16
  
  #pragma HLS pipeline II=6
  //#pragma HLS inline
  #pragma HLS array_partition variable=in1 complete
  #pragma HLS array_partition variable=in2 complete
  #pragma HLS array_partition variable=sorted_out complete
  
  T bitonic_in[2*NL1_EGOUT];
  get_bitonic_sequence_1(in1, in2, bitonic_in);
  hybridBitonicSort::bitonicMerger<T, 2*NL1_EGOUT, 0>::run(bitonic_in, 0);
  
  for(int i = 0; i < NL1_EGOUT; i++) {
    sorted_out[i] = bitonic_in[i];
  }
}
////////////////////////////////////////////////////////////////////////////////////

// MAIN SORTER (CALLED FROM TOP FUNCTION) //////////////////////////////////////////
template <typename T>
void pftkegsorter_barrel(bool newEvent, bool lastregion, bool region_odd,
			 const T objs_in[NOBJ], 
			 T objs_out[NL1_EGOUT]) {
  //#pragma HLS inline
  #pragma HLS array_partition variable=objs_in complete
  #pragma HLS array_partition variable=objs_out complete

  static T merge_merge_objs[NL1_EGOUT];
  #pragma HLS ARRAY_PARTITION variable=merge_merge_objs complete  
  if (newEvent) for(int i = 0; i < NL1_EGOUT; i++) merge_merge_objs[i].clear();
  //for (const auto &tmp : merge_merge_objs) std::cout<<"merge_merge_objs new "<<tmp.hwPt<<" "<<tmp.hwEta<<"\n";

  static T objs_in_0[NOBJ], objs_in_1[NOBJ];
  if (region_odd == false) for(int i = 0; i < NOBJ; i++) objs_in_0[i] = objs_in[i];
  else if (region_odd == true) { for(int i = 0; i < NOBJ; i++) objs_in_1[i] = objs_in[i];
  
    T merge_objs[NL1_EGOUT];
    #pragma HLS ARRAY_PARTITION variable=merge_objs complete
    merge_sort(objs_in_0, objs_in_1, merge_objs);//10, 10, 16
    //for (const auto &tmp : objs_in_0) std::cout<<"objs_in_0 "<<tmp.hwPt<<" "<<tmp.hwEta<<"\n";
    //for (const auto &tmp : objs_in_1) std::cout<<"objs_in_1 "<<tmp.hwPt<<" "<<tmp.hwEta<<"\n";    
    //for (const auto &tmp : merge_objs) std::cout<<"merge_objs "<<tmp.hwPt<<" "<<tmp.hwEta<<"\n";
    
    T merge_merge_objs_tmp[NL1_EGOUT];
    T merge_objs_tmp[NL1_EGOUT];
    for(int i = 0; i < NOBJ; i++) { merge_merge_objs_tmp[i] = merge_merge_objs[i];
      merge_objs_tmp[i] = merge_objs[i];
    }
    merge_sort(merge_merge_objs_tmp, merge_objs_tmp, merge_merge_objs);//16, 16, 16 ===> 10, 10, 16
  }
  if (lastregion) {
    T merge_merge_objs_tmp[NL1_EGOUT];
    for(int i = 0; i < NOBJ; i++) merge_merge_objs_tmp[i] = merge_merge_objs[i]; 
    merge_sort(merge_merge_objs_tmp, objs_in, merge_merge_objs);//10, 10, 16
  }
  
 fill_loop: for(unsigned int i = 0; i < NL1_EGOUT; i++) {
    #pragma HLS unroll
    objs_out[i] = merge_merge_objs[i];
    //std::cout<<"objs_out "<<objs_out[i].hwPt<<" "<<objs_out[i].hwEta<<"\n";
  }
}
////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
template <typename T>
void pftkegsorter_barrel_pack_in(const T objs_in[NOBJ],
                        ap_uint<T::BITWIDTH> packed_objs_in[NOBJ]) {
  #pragma HLS ARRAY_PARTITION variable=objs_in complete
  #pragma HLS ARRAY_PARTITION variable=packed_objs_in complete
  #pragma HLS inline
  #pragma HLS inline region recursive
  
  l1pf_pattern_pack<NOBJ>(objs_in, packed_objs_in);
}

template <typename T>
void pftkegsorter_barrel_unpack_in(const ap_uint<T::BITWIDTH> packed_objs_in[NOBJ], 
                          T objs_in[NOBJ]) {
  #pragma HLS ARRAY_PARTITION variable=packed_objs_in complete
  #pragma HLS ARRAY_PARTITION variable=objs_in complete
  #pragma HLS inline
  #pragma HLS inline region recursive
                          
  l1pf_pattern_unpack<NOBJ>(packed_objs_in, objs_in);
}

template <typename T>
void pftkegsorter_barrel_pack_out(const T objs_out[NL1_EGOUT], 
                         ap_uint<T::BITWIDTH> packed_objs_out[NL1_EGOUT]) {
  #pragma HLS ARRAY_PARTITION variable=objs_out complete
  #pragma HLS ARRAY_PARTITION variable=packed_objs_out complete
  #pragma HLS inline
  #pragma HLS inline region recursive
  l1pf_pattern_pack<NL1_EGOUT>(objs_out, packed_objs_out);
}

template <typename T>
void pftkegsorter_barrel_unpack_out(const ap_uint<T::BITWIDTH> packed_objs_out[NL1_EGOUT], 
                           T objs_out[NL1_EGOUT]) {
  #pragma HLS ARRAY_PARTITION variable=objs_out complete
  #pragma HLS ARRAY_PARTITION variable=packed_objs_out complete
  #pragma HLS inline
  #pragma HLS inline region recursive
  l1pf_pattern_unpack<NL1_EGOUT>(packed_objs_out, objs_out);
}
////////////////////////////////////////////////////////////////////////////////////

// TOP FUNCTION ////////////////////////////////////////////////////////////////////
void packed_pftkegsorter_barrel_pho(bool newEvent, bool lastregion, bool region_odd, 
		       const ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_objs_in[NOBJ],
                       ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_objs_out[NL1_EGOUT]);

void packed_pftkegsorter_barrel_ele(bool newEvent, bool lastregion, bool region_odd,
		       const ap_uint<l1ct::EGIsoEleObj::BITWIDTH> packed_objs_in[NOBJ],
                       ap_uint<l1ct::EGIsoEleObj::BITWIDTH> packed_objs_out[NL1_EGOUT]);
////////////////////////////////////////////////////////////////////////////////////
#endif

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
void merge_sort(const T in1[NOBJ], const T in2[NOBJ], T sorted_out[NL1_EGOUT]) {//10, 10, 16
  
  #pragma HLS pipeline II=6
  #pragma HLS array_partition variable=in1 complete
  #pragma HLS array_partition variable=in2 complete
  #pragma HLS array_partition variable=sorted_out complete
  
  T bitonic_in[2*NOBJ];
  get_bitonic_sequence(in1, in2, bitonic_in);
  hybridBitonicSort::bitonicMerger<T, 2*NOBJ, 0>::run(bitonic_in, 0);
  
  for(int i = 0; i < NL1_EGOUT; i++) {
    #pragma HLS UNROLL
    sorted_out[i] = bitonic_in[i];
  }
}
////////////////////////////////////////////////////////////////////////////////////

// MAIN SORTER (CALLED FROM TOP FUNCTION) //////////////////////////////////////////
template <typename T>
void pftkegsorter_barrel(bool newEvent, bool lastregion,
			 const T objs_in[NOBJ], 
			 T objs_out[NL1_EGOUT]) {
  #pragma HLS inline
  #pragma HLS array_partition variable=objs_in complete
  #pragma HLS array_partition variable=objs_out complete
    
  /////////////////////////////////////////////
  static T merge_merge_objs[NL1_EGOUT];//empty for newEvent == 1, and then 01, 0123, 01234 and so on 
  static unsigned int regionindex;//zero for newEvent == 1, and then 1, 2, 3, 4 and so on
  #pragma HLS ARRAY_PARTITION variable=merge_merge_objs complete
  if (newEvent) {
    for(int i = 0; i < NL1_EGOUT; i++) {
      #pragma HLS unroll
      merge_merge_objs[i].clear();//empty for newEvent == 1; size 16
    }
    regionindex = 0;//zero for newEvent == 1
  }
  /////////////////////////////////////////////

  /////////////////////////////////////////////
  static T objs_in_0[NOBJ], objs_in_1[NOBJ];//to store 10 objects in regions 0, 2, 4, 6 and so on
  #pragma HLS ARRAY_PARTITION variable=objs_in_0 complete
  if (regionindex %2 == 0) {//if region is 0, 2, 4 and so on
    for(int i = 0; i < NOBJ; i++) {
      #pragma HLS unroll
      objs_in_0[i] = objs_in[i];//10 objects stire for regions 0, 2, 4, 6 and so on
    }
  }
  else {//if region is 1, 3, 5 and so on
    for(int i = 0; i < NOBJ; i++) {
      #pragma HLS unroll
      objs_in_1[i] = objs_in[i];//10 objects stire for regions 0, 2, 4, 6 and so on
    }
    T merge_objs[NL1_EGOUT];
    #pragma HLS ARRAY_PARTITION variable=merge_objs complete
    merge_sort(objs_in_0, objs_in_1, merge_objs);//10, 10, 16 //merge regions 01, then 23, then 45 and so on
    
    merge_sort(merge_merge_objs, merge_objs, merge_merge_objs);//10, 10, 16 //merge empty and regions 01, then 01 and 23, then 0123 and 45 and so on
  }
  regionindex++;
  /////////////////////////////////////////////

  /////////////////////////////////////////////
  if (lastregion) {//if total #regions is odd number (e.g. 9 for HGCal)
    merge_sort(merge_merge_objs, objs_in, merge_merge_objs);//10, 10, 16 //merge 01 and 2 if total #regions is 3; merge 0123 and 4 if total #regions is 5 and so on
  }
  /////////////////////////////////////////////
  
 fill_loop: for(unsigned int i = 0; i < NL1_EGOUT; i++) {//output 16 objects
    #pragma HLS unroll
    objs_out[i] = merge_merge_objs[i];
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
void packed_pftkegsorter_barrel_pho(bool newEvent, bool lastregion,
		       const ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_objs_in[NOBJ],
                       ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_objs_out[NL1_EGOUT]);

void packed_pftkegsorter_barrel_ele(bool newEvent, bool lastregion,
		       const ap_uint<l1ct::EGIsoEleObj::BITWIDTH> packed_objs_in[NOBJ],
                       ap_uint<l1ct::EGIsoEleObj::BITWIDTH> packed_objs_out[NL1_EGOUT]);
////////////////////////////////////////////////////////////////////////////////////
#endif

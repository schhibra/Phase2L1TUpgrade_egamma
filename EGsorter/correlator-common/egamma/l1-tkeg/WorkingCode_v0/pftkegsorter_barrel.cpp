#include "pftkegsorter_barrel.h"

void packed_l2egsorter(bool newEvent, bool oddregion,
		       const ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_photons_in[NL1EGBOARDS][NL1_EGOUT],
                       const ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_eles_in[NL1EGBOARDS][NL1_EGOUT],//<=========== EGIsoEleObj
                       ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_photons_out[NL2_EGOUT],
                       ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_eles_out[NL2_EGOUT]){//<=========== EGIsoEleObj
  #pragma HLS ARRAY_PARTITION DIM=0 variable=packed_photons_in complete
  #pragma HLS ARRAY_PARTITION DIM=0 variable=packed_eles_in complete
  #pragma HLS ARRAY_PARTITION variable=packed_photons_out complete
  #pragma HLS ARRAY_PARTITION variable=packed_eles_out complete

  #ifdef HLS_pipeline_II
    #if HLS_pipeline_II == 1
       #pragma HLS pipeline II=1
    #elif HLS_pipeline_II == 2
       #pragma HLS pipeline II=2
    #elif HLS_pipeline_II == 3
       #pragma HLS pipeline II=3
    #elif HLS_pipeline_II == 4
       #pragma HLS pipeline II=4
    #elif HLS_pipeline_II == 6
       #pragma HLS pipeline II=6
    #endif
  #else
    #pragma HLS pipeline II=6
  #endif

  l1ct::EGIsoObj photons_in[NL1EGBOARDS][NL1_EGOUT]; 
  l1ct::EGIsoObj eles_in[NL1EGBOARDS][NL1_EGOUT];//<=========== EGIsoEleObj
  l1ct::EGIsoObj photons_out[NL2_EGOUT];
  l1ct::EGIsoObj eles_out[NL2_EGOUT];//<=========== EGIsoEleObj
  
  #pragma HLS ARRAY_PARTITION DIM=0 variable=photons_in complete
  #pragma HLS ARRAY_PARTITION DIM=0 variable=eles_in complete
  #pragma HLS ARRAY_PARTITION variable=photons_out complete
  #pragma HLS ARRAY_PARTITION variable=eles_out complete
  
  l2egsorter_unpack_in(packed_photons_in, packed_eles_in, photons_in, eles_in);
  l2egsorter(newEvent, oddregion, photons_in, eles_in, photons_out, eles_out);
  l2egsorter_pack_out(photons_out, eles_out, packed_photons_out, packed_eles_out);
                         
}
////////////////////////////////////////////////////////////////////////////////////
void l2egsorter_pack_out(const l1ct::EGIsoObj photons_out[NL2_EGOUT], 
                         const l1ct::EGIsoObj eles_out[NL2_EGOUT],//<=========== EGIsoEleObj
                         ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_photons_out[NL2_EGOUT], 
                         ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_eles_out[NL2_EGOUT]) {//<=========== EGIsoEleObj
  l2egsorter_pho_pack_out(photons_out, packed_photons_out);
  l2egsorter_ele_pack_out(eles_out, packed_eles_out);
}

void l2egsorter_pho_pack_out(const l1ct::EGIsoObj photons_out[NL2_EGOUT], 
                             ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_photons_out[NL2_EGOUT]) {
  #pragma HLS ARRAY_PARTITION variable=photons_out complete
  #pragma HLS ARRAY_PARTITION variable=packed_photons_out complete
  #pragma HLS inline
  #pragma HLS inline region recursive
  l1pf_pattern_pack<NL2_EGOUT>(photons_out, packed_photons_out);
}

void l2egsorter_ele_pack_out(const l1ct::EGIsoObj eles_out[NL2_EGOUT],//<=========== EGIsoEleObj
                             ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_eles_out[NL2_EGOUT]) {//<=========== EGIsoEleObj
  #pragma HLS ARRAY_PARTITION variable = eles_out complete
  #pragma HLS ARRAY_PARTITION variable = packed_eles_out complete
  #pragma HLS inline
  #pragma HLS inline region recursive
  l1pf_pattern_pack<NL2_EGOUT>(eles_out, packed_eles_out);
}
////////////////////////////////////////////////////////////////////////////////////

void l2egsorter_pack_in(const l1ct::EGIsoObj photons_in[NL1EGBOARDS][NL1_EGOUT], 
                        const l1ct::EGIsoObj eles_in[NL1EGBOARDS][NL1_EGOUT],//<=========== EGIsoEleObj
                        ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_photons_in[NL1EGBOARDS][NL1_EGOUT], 
                        ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_eles_in[NL1EGBOARDS][NL1_EGOUT]) {//<=========== EGIsoEleObj
  #pragma HLS ARRAY_PARTITION DIM=0 variable=photons_in complete
  #pragma HLS ARRAY_PARTITION DIM=0 variable=eles_in complete
  #pragma HLS ARRAY_PARTITION DIM=0 variable=packed_photons_in complete
  #pragma HLS ARRAY_PARTITION DIM=0 variable=packed_eles_in complete
  #pragma HLS inline
  #pragma HLS inline region recursive
  
  for(unsigned int board = 0; board < NL1EGBOARDS; board++) {
    #pragma HLS UNROLL
    l1pf_pattern_pack<NL1_EGOUT>(photons_in[board], packed_photons_in[board]);
    l1pf_pattern_pack<NL1_EGOUT>(eles_in[board], packed_eles_in[board]);           
  }           
}
////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void get_bitonic_sequence(const T in_board1[NL1_EGOUT], const T in_board2[NL1_EGOUT], T bitonic_in[2*NL1_EGOUT]) {
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
void merge_sort(const T in1[NL1_EGOUT], const T in2[NL1_EGOUT], T sorted_out[NL1_EGOUT]) {
  
  #pragma HLS inline
  #pragma HLS array_partition variable=in1 complete
  #pragma HLS array_partition variable=in2 complete
  #pragma HLS array_partition variable=sorted_out complete

  T bitonic_in[2*NL1_EGOUT];
  get_bitonic_sequence(in1, in2, bitonic_in);
  hybridBitonicSort::bitonicMerger<T, 2*NL1_EGOUT, 0>::run(bitonic_in, 0);

  for(int i = 0; i < NL1_EGOUT; i++) {
    sorted_out[i] = bitonic_in[i];
  }
}
////////////////////////////////////////////////////////////////////////////////////

void l2egsorter(bool newEvent, bool oddregion,
		const l1ct::EGIsoObj photons_in[NL1EGBOARDS][NL1_EGOUT], 
                const l1ct::EGIsoObj eles_in[NL1EGBOARDS][NL1_EGOUT],//<=========== EGIsoEleObj
                l1ct::EGIsoObj photons_out[NL2_EGOUT], 
                l1ct::EGIsoObj eles_out[NL2_EGOUT]) {//<=========== EGIsoEleObj
  #pragma HLS array_partition DIM=0 variable=photons_in complete
  #pragma HLS array_partition DIM=0 variable=eles_in complete
  #pragma HLS array_partition variable=photons_out complete
  #pragma HLS array_partition variable=eles_out complete

  l1ct::EGIsoObj merge_photons[NL1_EGOUT];
  if (!oddregion) merge_sort(photons_in[0], photons_in[1], merge_photons);
  //for (const auto &tmp : photons_in[0]) std::cout<<"photons_in[0] "<<tmp.hwPt<<" "<<tmp.hwEta<<"\n";
  //for (const auto &tmp : photons_in[1]) std::cout<<"photons_in[1] "<<tmp.hwPt<<" "<<tmp.hwEta<<"\n";    
  //for (const auto &tmp : merge_photons) std::cout<<"merge_photons "<<tmp.hwPt<<" "<<tmp.hwEta<<"\n";
    
  l1ct::EGIsoObj merge_eles[NL1_EGOUT];//<=========== EGIsoEleObj
  if (!oddregion) merge_sort(eles_in[0], eles_in[1], merge_eles);


  
  static l1ct::EGIsoObj merge_mrge_photons[NL1_EGOUT];
  static l1ct::EGIsoObj merge_mrge_eles[NL1_EGOUT];//<=========== EGIsoEleObj
  #pragma HLS ARRAY_PARTITION variable=merge_mrge_photons complete
  #pragma HLS ARRAY_PARTITION variable=merge_mrge_eles complete
  //for (const auto &tmp : merge_mrge_photons) std::cout<<"merge_mrge_photons "<<tmp.hwPt<<" "<<tmp.hwEta<<"\n";
  
  if (newEvent) {
    for(int i = 0; i < NL1_EGOUT; i++) { merge_mrge_photons[i].clear(); merge_mrge_eles[i].clear();}
    //for (const auto &tmp : merge_mrge_photons) std::cout<<"merge_mrge_photons new "<<tmp.hwPt<<" "<<tmp.hwEta<<"\n";
  }
  
  if (!oddregion) {
    merge_sort(merge_mrge_photons, merge_photons, merge_mrge_photons);
    merge_sort(merge_mrge_photons, merge_eles, merge_mrge_eles);
  }
  else {
    merge_sort(merge_mrge_photons, photons_in[0], merge_mrge_photons);
    merge_sort(merge_mrge_photons, eles_in[0], merge_mrge_eles);
  }
  
 fill_loop: for(unsigned int io = 0; io < NL1_EGOUT; io++) {
    #pragma HLS unroll
    photons_out[io] = merge_mrge_photons[io];
    eles_out[io] = merge_mrge_eles[io];
    //std::cout<<"photons_out "<<photons_out[io].hwPt<<" "<<photons_out[io].hwEta<<"\n";
  }
}
////////////////////////////////////////////////////////////////////////////////////
void l2egsorter_unpack_in(const ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_photons_in[NL1EGBOARDS][NL1_EGOUT], 
                          const ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_eles_in[NL1EGBOARDS][NL1_EGOUT],//<=========== EGIsoEleObj
                          l1ct::EGIsoObj photons_in[NL1EGBOARDS][NL1_EGOUT], 
                          l1ct::EGIsoObj eles_in[NL1EGBOARDS][NL1_EGOUT]) {//<=========== EGIsoEleObj
  l2egsorter_pho_unpack_in(packed_photons_in, photons_in);
  l2egsorter_ele_unpack_in(packed_eles_in, eles_in);                      
}

void l2egsorter_pho_unpack_in(const ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_photons_in[NL1EGBOARDS][NL1_EGOUT], 
                              l1ct::EGIsoObj photons_in[NL1EGBOARDS][NL1_EGOUT]) {
  #pragma HLS ARRAY_PARTITION DIM=0 variable=packed_photons_in complete
  #pragma HLS ARRAY_PARTITION DIM=0 variable=photons_in complete
  #pragma HLS inline
  #pragma HLS inline region recursive
                          
  for(unsigned int board = 0; board < NL1EGBOARDS; board++) {
    #pragma HLS UNROLL
    l1pf_pattern_unpack<NL1_EGOUT>(packed_photons_in[board], photons_in[board]);
  }                              
}

void l2egsorter_ele_unpack_in(const ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_eles_in[NL1EGBOARDS][NL1_EGOUT],//<=========== EGIsoEleObj
                              l1ct::EGIsoObj eles_in[NL1EGBOARDS][NL1_EGOUT]) {//<=========== EGIsoEleObj
  #pragma HLS ARRAY_PARTITION DIM=0 variable=packed_eles_in complete
  #pragma HLS ARRAY_PARTITION DIM=0 variable=eles_in complete
  #pragma HLS inline
  #pragma HLS inline region recursive
                          
  for(unsigned int board = 0; board < NL1EGBOARDS; board++) {
    #pragma HLS UNROLL
    l1pf_pattern_unpack<NL1_EGOUT>(packed_eles_in[board], eles_in[board]);           
  }                                
}
////////////////////////////////////////////////////////////////////////////////////

void l2egsorter_unpack_out(const ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_photons_out[NL2_EGOUT], 
                           const ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_eles_out[NL2_EGOUT],//<=========== EGIsoEleObj
                           l1ct::EGIsoObj photons_out[NL2_EGOUT], 
                           l1ct::EGIsoObj eles_out[NL2_EGOUT]) {//<=========== EGIsoEleObj
                             #pragma HLS ARRAY_PARTITION variable=photons_out complete
  #pragma HLS ARRAY_PARTITION variable=eles_out complete
  #pragma HLS ARRAY_PARTITION variable=packed_photons_out complete
  #pragma HLS ARRAY_PARTITION variable=packed_eles_out complete
  #pragma HLS inline
  #pragma HLS inline region recursive
  l1pf_pattern_unpack<NL2_EGOUT>(packed_photons_out, photons_out);
  l1pf_pattern_unpack<NL2_EGOUT>(packed_eles_out, eles_out);                                      
}

#ifndef FIRMWARE_PFTKEGALGO_H
#define FIRMWARE_PFTKEGALGO_H

#include "../../../dataformats/layer1_objs.h"
#include "../../../dataformats/layer1_emulator.h"

#define NOBJTOSORT 6
#define NOBJSORTED 54

template <typename T, unsigned int NOBJ>
void pftkegsorter(bool newEvent,
		  const T & eg,
		  T egs_sorted[NOBJ]){
  #pragma HLS PIPELINE ii=1
  #pragma HLS ARRAY_PARTITION variable=egs_sorted complete
  
  static T cells[NOBJ];
  #pragma HLS ARRAY_PARTITION variable=cells complete

  bool below[NOBJ];
  #pragma HLS ARRAY_PARTITION variable=below complete
  for (int i = 0; i < NOBJ; ++i) below[i] = !newEvent && (cells[i].hwPt <= eg.hwPt);

  for (int i = NOBJ-1; i >= 1; --i) {
    if      (below[i]) cells[i] = (below[i-1] ? cells[i-1] : eg);
    else if (newEvent) cells[i].hwPt = 0;
  }
  if (newEvent || below[0]) cells[0] = eg;
   
  for (unsigned int i = 0; i < NOBJ; ++i) {
  #pragma HLS unroll
    egs_sorted[i] = cells[i];
  }
  return;
}


template <typename T>
void pftkegsorter_pack_in(const T & eg,
              ap_uint<T::BITWIDTH> & packed_eg){
  #pragma HLS inline
  #pragma HLS inline region recursive
  l1pf_pattern_pack<1>(&eg, &packed_eg);
}

template <typename T>
void pftkegsorter_unpack_in(const ap_uint<T::BITWIDTH> & packed_eg,
			    T & eg){
  #pragma HLS inline
  #pragma HLS inline region recursive
  l1pf_pattern_unpack<1>(&packed_eg, &eg);
}

template <typename T, unsigned int NOBJ>
void pftkegsorter_pack_out(const T egs_sorted[NOBJ],
			   ap_uint<T::BITWIDTH> packed_egs_sorted[NOBJ]){
  #pragma HLS ARRAY_PARTITION variable=egs_sorted complete
  #pragma HLS ARRAY_PARTITION variable=packed_egs_sorted complete
  #pragma HLS inline
  #pragma HLS inline region recursive
  l1pf_pattern_pack<NOBJ>(egs_sorted, packed_egs_sorted);
}

template <typename T, unsigned int NOBJ>
void pftkegsorter_unpack_out(const ap_uint<T::BITWIDTH> packed_egs_sorted[NOBJ],
                 T egs_sorted[NOBJ]){
  #pragma HLS ARRAY_PARTITION variable=packed_egs_sorted complete
  #pragma HLS ARRAY_PARTITION variable=egs_sorted complete
  #pragma HLS inline
  #pragma HLS inline region recursive                        
  l1pf_pattern_unpack<NOBJ>(packed_egs_sorted, egs_sorted);
}



void pftkegsorter_pho(bool newEvent,
		  const l1ct::EGIsoObj photon,
		  l1ct::EGIsoObj photons_sorted[NOBJSORTED]);

void pftkegsorter_ele(bool newEvent,
		  const l1ct::EGIsoEleObj electron,
		  l1ct::EGIsoEleObj electrons_sorted[NOBJSORTED]);


void packed_pftkegsorter_ele(bool newEvent,
		  const ap_uint<l1ct::EGIsoEleObj::BITWIDTH> packed_electron,
		  ap_uint<l1ct::EGIsoEleObj::BITWIDTH> packed_electrons_sorted[NOBJSORTED]);

void packed_pftkegsorter_pho(bool newEvent,
		  const ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_photon,
		  ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_photons_sorted[NOBJSORTED]);
#endif

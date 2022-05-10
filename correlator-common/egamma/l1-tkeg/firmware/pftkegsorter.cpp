#include "pftkegsorter.h"
#include "../../../dataformats/bit_encoding.h"

void pftkegsorter_pho(bool newEvent,
		  const l1ct::EGIsoObj photon,
		  l1ct::EGIsoObj photons_sorted[NOBJSORTED]){
  #pragma HLS PIPELINE ii=1
  #pragma HLS ARRAY_PARTITION variable=photons_sorted complete

  pftkegsorter<l1ct::EGIsoObj,NOBJSORTED>(newEvent, photon, photons_sorted);
}

void packed_pftkegsorter_pho(bool newEvent,
		  const ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_photon,
		  ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_photons_sorted[NOBJSORTED]){
  #pragma HLS PIPELINE ii=1
  #pragma HLS ARRAY_PARTITION variable=packed_photons_sorted complete

  l1ct::EGIsoObj photon;
  l1ct::EGIsoObj photons_sorted[NOBJSORTED];
  #pragma HLS ARRAY_PARTITION variable=photons_sorted complete dim=1

  pftkegsorter_unpack_in<l1ct::EGIsoObj>(packed_photon, photon);
  pftkegsorter<l1ct::EGIsoObj,NOBJSORTED>(newEvent, photon, photons_sorted);
  pftkegsorter_pack_out<l1ct::EGIsoObj,NOBJSORTED>(photons_sorted, packed_photons_sorted);
}

void pftkegsorter_ele(bool newEvent,
		  const l1ct::EGIsoEleObj electron,
		  l1ct::EGIsoEleObj electrons_sorted[NOBJSORTED]){
  #pragma HLS PIPELINE ii=1
  #pragma HLS ARRAY_PARTITION variable=electrons_sorted complete

  pftkegsorter<l1ct::EGIsoEleObj,NOBJSORTED>(newEvent, electron, electrons_sorted);
}

void packed_pftkegsorter_ele(bool newEvent,
		  const ap_uint<l1ct::EGIsoEleObj::BITWIDTH> packed_electron,
		  ap_uint<l1ct::EGIsoEleObj::BITWIDTH> packed_electrons_sorted[NOBJSORTED]){
  #pragma HLS PIPELINE ii=1
  #pragma HLS ARRAY_PARTITION variable=packed_electrons_sorted complete

  l1ct::EGIsoEleObj electron;
  l1ct::EGIsoEleObj electrons_sorted[NOBJSORTED];
  #pragma HLS ARRAY_PARTITION variable=electrons_sorted complete dim=1

  pftkegsorter_unpack_in<l1ct::EGIsoEleObj>(packed_electron, electron);
  pftkegsorter<l1ct::EGIsoEleObj,NOBJSORTED>(newEvent, electron, electrons_sorted);
  pftkegsorter_pack_out<l1ct::EGIsoEleObj,NOBJSORTED>(electrons_sorted, packed_electrons_sorted);
}

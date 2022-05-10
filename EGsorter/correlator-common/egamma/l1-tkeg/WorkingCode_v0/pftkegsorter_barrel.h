#ifndef FIRMWARE_PFTKEGSORTER_BARREL_H
#define FIRMWARE_PFTKEGSORTER_BARREL_H

#include "../../../dataformats/layer1_multiplicities.h"
#include "../../../dataformats/layer1_emulator.h"
#include "../../../common/firmware/bitonic_hybrid.h"

#define NL1EGBOARDS 2
#define NL1_EGOUT 16
#define NL2_EGOUT 16

void l2egsorter(bool newEvent, bool oddregion,
		const l1ct::EGIsoObj photons_in[NL1EGBOARDS][NL1_EGOUT], 
                const l1ct::EGIsoObj eles_in[NL1EGBOARDS][NL1_EGOUT],//<=========== EGIsoEleObj
                l1ct::EGIsoObj photons_out[NL2_EGOUT], 
                l1ct::EGIsoObj eles_out[NL2_EGOUT]);//<=========== EGIsoEleObj

void l2egsorter_pack_in(const l1ct::EGIsoObj photons_in[NL1EGBOARDS][NL1_EGOUT],
                        const l1ct::EGIsoObj eles_in[NL1EGBOARDS][NL1_EGOUT],//<=========== EGIsoEleObj
                        ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_photons_in[NL1EGBOARDS][NL1_EGOUT],
                        ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_eles_in[NL1EGBOARDS][NL1_EGOUT]);//<=========== EGIsoEleObj
////////////////////////////////////////////////////////////////////////////////////
void l2egsorter_pack_out(const l1ct::EGIsoObj photons_out[NL2_EGOUT], 
                         const l1ct::EGIsoObj eles_out[NL2_EGOUT],//<=========== EGIsoEleObj
                         ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_photons_out[NL2_EGOUT], 
                         ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_eles_out[NL2_EGOUT]);//<=========== EGIsoEleObj

void l2egsorter_pho_pack_out(const l1ct::EGIsoObj photons_out[NL2_EGOUT], 
                             ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_photons_out[NL2_EGOUT]);

void l2egsorter_ele_pack_out(const l1ct::EGIsoObj eles_out[NL2_EGOUT],//<=========== EGIsoEleObj
                             ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_eles_out[NL2_EGOUT]);//<=========== EGIsoEleObj
////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
void packed_l2egsorter(bool newEvent, bool oddregion,
		       const ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_photons_in[NL1EGBOARDS][NL1_EGOUT],
                       const ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_eles_in[NL1EGBOARDS][NL1_EGOUT],//<=========== EGIsoEleObj
                       ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_photons_out[NL2_EGOUT],
                       ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_eles_out[NL2_EGOUT]);//<=========== EGIsoEleObj
////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
void l2egsorter_unpack_in(const ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_photons_in[NL1EGBOARDS][NL1_EGOUT],
                          const ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_eles_in[NL1EGBOARDS][NL1_EGOUT],//<=========== EGIsoEleObj
                          l1ct::EGIsoObj photons_in[NL1EGBOARDS][NL1_EGOUT],
                          l1ct::EGIsoObj eles_in[NL1EGBOARDS][NL1_EGOUT]);//<=========== EGIsoEleObj

void l2egsorter_pho_unpack_in(const ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_photons_in[NL1EGBOARDS][NL1_EGOUT],
                              l1ct::EGIsoObj photons_in[NL1EGBOARDS][NL1_EGOUT]);

void l2egsorter_ele_unpack_in(const ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_eles_in[NL1EGBOARDS][NL1_EGOUT],//<=========== EGIsoEleObj
                              l1ct::EGIsoObj eles_in[NL1EGBOARDS][NL1_EGOUT]);//<=========== EGIsoEleObj
////////////////////////////////////////////////////////////////////////////////////

void l2egsorter_unpack_out(const ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_photons_out[NL2_EGOUT], 
                           const ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_eles_out[NL2_EGOUT],//<=========== EGIsoEleObj
                           l1ct::EGIsoObj photons_out[NL2_EGOUT], 
                           l1ct::EGIsoObj eles_out[NL2_EGOUT]);//<=========== EGIsoEleObj

#endif

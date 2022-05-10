#ifndef FIRMWARE_PFTKEGALGO_H
#define FIRMWARE_PFTKEGALGO_H

#include "../../../dataformats/layer1_objs.h"
#include "../../../dataformats/pf.h"
#include "../../../dataformats/egamma.h"
#include "../../../dataformats/layer1_multiplicities.h"

void pftkegalgo(const l1ct::PFRegion & region, 
                const l1ct::EmCaloObj emcalo[NEMCALO_EGIN], 
                const l1ct::TkObj track[NTRACK_EGIN],
                l1ct::EGIsoObj photons[NEM_EGOUT], 
                l1ct::EGIsoEleObj eles[NEM_EGOUT]);

void packed_pftkegalgo(const ap_uint<l1ct::PFRegion::BITWIDTH> & packed_region, 
                       const ap_uint<l1ct::EmCaloObj::BITWIDTH> packed_emcalo[NEMCALO], 
                       const ap_uint<l1ct::TkObj::BITWIDTH> packed_track[NTRACK],
                       ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_photons[NEM_EGOUT], 
                       ap_uint<l1ct::EGIsoEleObj::BITWIDTH> packed_eles[NEM_EGOUT]);

void pftkegalgo_pack_in(const l1ct::PFRegion & region, 
                        const l1ct::EmCaloObj emcalo[NEMCALO], 
                        const l1ct::TkObj track[NTRACK],
                        ap_uint<l1ct::PFRegion::BITWIDTH> & packed_region, 
                        ap_uint<l1ct::EmCaloObj::BITWIDTH> packed_emcalo[NEMCALO], 
                        ap_uint<l1ct::TkObj::BITWIDTH> packed_track[NTRACK]);
                         
void pftkegalgo_unpack_in(const ap_uint<l1ct::PFRegion::BITWIDTH> & packed_region, 
                          const ap_uint<l1ct::EmCaloObj::BITWIDTH> packed_emcalo[NEMCALO], 
                          const ap_uint<l1ct::TkObj::BITWIDTH> packed_track[NTRACK],
                          l1ct::PFRegion & region, 
                          l1ct::EmCaloObj emcalo[NEMCALO], 
                          l1ct::TkObj track[NTRACK]);
 
void pftkegalgo_pack_out(const l1ct::EGIsoObj photons[NEM_EGOUT], 
                         const l1ct::EGIsoEleObj eles[NEM_EGOUT],
                         ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_photons[NEM_EGOUT], 
                         ap_uint<l1ct::EGIsoEleObj::BITWIDTH> packed_eles[NEM_EGOUT]);

void pftkegalgo_unpack_out(const ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_photons[NEM_EGOUT], 
                           const ap_uint<l1ct::EGIsoEleObj::BITWIDTH> packed_eles[NEM_EGOUT],
                           l1ct::EGIsoObj photons[NEM_EGOUT], 
                           l1ct::EGIsoEleObj eles[NEM_EGOUT]);

void link_emCalo2tk(const l1ct::PFRegion & region, const l1ct::EmCaloObj emcalo[NEMCALO_EGIN], const l1ct::TkObj track[NTRACK_EGIN], ap_uint<NTRACK_EGIN> emCalo2tk_bit[NEMCALO_EGIN]);

void link_emCalo2emCalo(const l1ct::EmCaloObj emcalo[NEMCALO_EGIN], ap_uint<NEMCALO_EGIN> emCalo2emcalo_bit[NEMCALO_EGIN]);

#endif

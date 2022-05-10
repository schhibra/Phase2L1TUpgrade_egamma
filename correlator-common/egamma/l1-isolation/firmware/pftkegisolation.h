#ifndef FIRMWARE_PFTKEGISOLATION_H
#define FIRMWARE_PFTKEGISOLATION_H

#include "../../../dataformats/layer1_objs.h"
#include "../../../dataformats/pf.h"
#include "../../../dataformats/egamma.h"
#include "../../../dataformats/layer1_multiplicities.h"

// FIXME: do we need the region???
void tkeg_tkisolation(const l1ct::PFRegion& region,
                      const l1ct::PVObj& pv,
                      const l1ct::TkObj track[NTRACK],
                      l1ct::EGIsoObj photons[NEM_EGOUT],
                      l1ct::EGIsoEleObj eles[NEM_EGOUT]);

void packed_tkeg_tkisolation(const ap_uint<l1ct::PFRegion::BITWIDTH>& p_region,
                             const ap_uint<l1ct::PVObj::BITWIDTH>& p_pv,
                             const ap_uint<l1ct::TkObj::BITWIDTH> p_track[NTRACK],
                             const ap_uint<l1ct::EGIsoObj::BITWIDTH> p_photons_in[NEM_EGOUT],
                             const ap_uint<l1ct::EGIsoEleObj::BITWIDTH> p_eles_in[NEM_EGOUT],
                             ap_uint<l1ct::EGIsoObj::BITWIDTH> p_photons_out[NEM_EGOUT],
                             ap_uint<l1ct::EGIsoEleObj::BITWIDTH> p_eles_out[NEM_EGOUT]);

void tkeg_tkisolation_pack_in(const l1ct::PFRegion& region,
                              const l1ct::PVObj& pv,
                              const l1ct::TkObj track[NTRACK],
                              const l1ct::EGIsoObj photons[NEM_EGOUT],
                              const l1ct::EGIsoEleObj eles[NEM_EGOUT],
                              ap_uint<l1ct::PFRegion::BITWIDTH>& p_region,
                              ap_uint<l1ct::PVObj::BITWIDTH>& p_pv,
                              ap_uint<l1ct::TkObj::BITWIDTH> p_track[NTRACK],
                              ap_uint<l1ct::EGIsoObj::BITWIDTH> p_photons[NEM_EGOUT],
                              ap_uint<l1ct::EGIsoEleObj::BITWIDTH> p_eles[NEM_EGOUT]);

void tkeg_tkisolation_unpack_in(const ap_uint<l1ct::PFRegion::BITWIDTH>& p_region,
                                const ap_uint<l1ct::PVObj::BITWIDTH>& p_pv,
                                const ap_uint<l1ct::TkObj::BITWIDTH> p_track[NTRACK],
                                const ap_uint<l1ct::EGIsoObj::BITWIDTH> p_photons[NEM_EGOUT],
                                const ap_uint<l1ct::EGIsoEleObj::BITWIDTH> p_eles[NEM_EGOUT],
                                l1ct::PFRegion& region,
                                l1ct::PVObj& pv,
                                l1ct::TkObj track[NTRACK],
                                l1ct::EGIsoObj photons[NEM_EGOUT],
                                l1ct::EGIsoEleObj eles[NEM_EGOUT]);

void tkeg_tkisolation_pack_out(const l1ct::EGIsoObj photons[NEM_EGOUT],
                               const l1ct::EGIsoEleObj eles[NEM_EGOUT],
                               ap_uint<l1ct::EGIsoObj::BITWIDTH> p_photons[NEM_EGOUT],
                               ap_uint<l1ct::EGIsoEleObj::BITWIDTH> p_eles[NEM_EGOUT]);

void tkeg_tkisolation_unpack_out(const ap_uint<l1ct::EGIsoObj::BITWIDTH> p_photons[NEM_EGOUT],
                                 const ap_uint<l1ct::EGIsoEleObj::BITWIDTH> p_eles[NEM_EGOUT],
                                 l1ct::EGIsoObj photons[NEM_EGOUT],
                                 l1ct::EGIsoEleObj eles[NEM_EGOUT]);

#endif

#include "pftkegisolation.h"
#include <cassert>
#include "hls_math.h"

// #include "../../../pf/firmware/pfalgo_types.h"

using namespace l1ct;

struct IsoParameters {
  IsoParameters(pt_t minpt, ap_int<z0_t::width + 1> dZ, int dRMin2, int dRMax2)
      : tkQualityPtMin(minpt), dZ(dZ), dRMin2(dRMin2), dRMax2(dRMax2) {}
  pt_t tkQualityPtMin;
  ap_int<z0_t::width + 1> dZ;
  int dRMin2;
  int dRMax2;
};

template <typename T>
int deltaR2(const T &charged, const EGIsoObj &egphoton) {
  // NOTE: not inlining these ones allows for DPS reusage for II != 1
  // while inlining yelds the best latency
  // # pragma HLS inline 
  // # pragma HLS inline region recursive
  // FIXME: we compare Tk at vertex against the calo variable...
  // return dr2_int(charged.hwEta, charged.hwPhi, egphoton.hwEta, egphoton.hwPhi)
  return dr2_int(charged.hwVtxEta(), charged.hwVtxPhi(), egphoton.hwEta, egphoton.hwPhi);
}

template <typename T>
int deltaR2(const T &charged, const EGIsoEleObj &egele) {
  // # pragma HLS inline

  ap_int<phi_t::width + 1> d_phi = (charged.hwVtxPhi() - egele.hwVtxPhi());
  ap_int<eta_t::width + 1> d_eta = (charged.hwVtxEta() - egele.hwVtxEta());
  return d_phi * d_phi + d_eta * d_eta;
  // return dr2_int(charged.hwVtxEta(), charged.hwVtxPhi(), egele.hwVtxEta(), egele.hwVtxPhi())

}

int deltaR2(const PFNeutralObj &neutral, const EGIsoObj &egphoton) {
  // # pragma HLS inline

  ap_int<phi_t::width + 1> d_phi = (neutral.hwPhi - egphoton.hwPhi);
  ap_int<eta_t::width + 1> d_eta = (neutral.hwEta - egphoton.hwEta);
  return d_phi * d_phi + d_eta * d_eta;
}

int deltaR2(const PFNeutralObj &neutral, const EGIsoEleObj &egele) {
  // # pragma HLS inline

  // FIXME: we compare Tk at vertex against the calo variable...
  // ap_int<phi_t::width + 1> d_phi = (neutral.hwPhi - egele.hwPhi);
  // ap_int<eta_t::width + 1> d_eta = (neutral.hwEta - egele.hwEta);
  ap_int<phi_t::width + 1> d_phi = (neutral.hwPhi - egele.hwVtxPhi());
  ap_int<eta_t::width + 1> d_eta = (neutral.hwEta - egele.hwVtxEta());
  return d_phi * d_phi + d_eta * d_eta;
}

template <typename T>
ap_int<z0_t::width + 1> deltaZ0(const T &charged, const EGIsoObj &egphoton, z0_t z0) {
  # pragma HLS inline region recursive
  # pragma HLS inline 

  return hls::abs(charged.hwZ0 - z0);
}

template <typename T>
ap_int<z0_t::width + 1> deltaZ0(const T &charged, const EGIsoEleObj &egele, z0_t z0) {
  // # pragma HLS inline region recursive
  # pragma HLS inline 
  # pragma HLS inline region recursive
  return hls::abs(charged.hwZ0 - egele.hwZ0);
}

template <typename TCH, int NCH, typename TEG>
void compute_sumPt(
    iso_t &sumPtPV, const TCH objects[NCH], const TEG &egobj, const IsoParameters &params, const z0_t z0) {

#pragma HLS ARRAY_PARTITION variable=objects complete dim=1
#pragma HLS INLINE

  iso_loop_charged: for (int itk = 0; itk < NCH; ++itk) {
    const auto &obj = objects[itk];
    int dR2 = deltaR2(obj, egobj);

    bool use = (obj.hwPt >= params.tkQualityPtMin) &&
               (dR2 > params.dRMin2) && 
               (dR2 < params.dRMax2) &&
               (deltaZ0(obj, egobj, z0) < params.dZ);
    iso_t maybe_pt = use ? iso_t(obj.hwPt) : iso_t(0);
    sumPtPV += maybe_pt;
  }
}

template <typename TEG>
void compute_isolation(TEG egobjs[NEM_EGOUT],
                       const l1ct::TkObj tracks[NTRACK],
                       const IsoParameters &params,
                       const z0_t z0) {
#pragma HLS ARRAY_PARTITION variable=egobjs complete dim=1
#pragma HLS ARRAY_PARTITION variable=tracks complete dim=1
       
  iso_loop_egobjs: for (int ic = 0; ic < NEM_EGOUT; ++ic) {
    auto &egobj = egobjs[ic];
    if (egobj.hwPt == 0) continue;
    iso_t sumPtPV = 0.;
    // bool debug = false;
    // if (egobj.hwPt == 3.50 && egobj.hwEta == +23) debug = true;
    // if (egobj.hwPt == 38.25 && egobj.hwEta == +54) debug = true;
    // if(debug) {
    //   std::cout << "[FW] ele pt: " << egobj.hwPt << " eta: " << egobj.hwEta << " phi: " << egobj.hwPhi << std::endl;
    // }
    compute_sumPt<l1ct::TkObj, NTRACK, TEG>(sumPtPV, tracks, egobj, params, z0);
    egobj.hwIso = sumPtPV;
  }
}

void tkeg_tkisolation(const l1ct::PFRegion &region,
                      const l1ct::PVObj &pv,
                      const l1ct::TkObj tracks[NTRACK],
                      l1ct::EGIsoObj photons[NEM_EGOUT],
                      l1ct::EGIsoEleObj eles[NEM_EGOUT]) {
#pragma HLS ARRAY_PARTITION variable=tracks complete dim=1
#pragma HLS ARRAY_PARTITION variable=photons complete dim=1
#pragma HLS ARRAY_PARTITION variable=eles complete dim=1

// FIXME: configure pipeline II
#ifdef HLS_pipeline_II
  #if HLS_pipeline_II == 1
    #pragma HLS pipeline II = 1
  #elif HLS_pipeline_II == 2
    #pragma HLS pipeline II = 2
  #elif HLS_pipeline_II == 3
    #pragma HLS pipeline II = 3
  #elif HLS_pipeline_II == 4
    #pragma HLS pipeline II = 4
  #elif HLS_pipeline_II == 6
    #pragma HLS pipeline II = 6
  #endif
#else
  #pragma HLS pipeline II = 2
#endif
// #pragma HLS pipeline II=1

  // cfg.tkIsoParams_tkEle = {2., 0.6, 0.03, 0.2};
  static const IsoParameters params_tkiso_ele(2., 12, 48, 2101);
  // cfg.tkIsoParams_tkEm = {2., 0.6, 0.07, 0.3};
  static const IsoParameters params_tkiso_ph(2., 12, 258, 4728);
  
  compute_isolation(photons, tracks, params_tkiso_ph, pv.hwZ0);
  compute_isolation(eles, tracks, params_tkiso_ele, pv.hwZ0);
}

void packed_tkeg_tkisolation(const ap_uint<l1ct::PFRegion::BITWIDTH>& p_region,
                             const ap_uint<l1ct::PVObj::BITWIDTH>& p_pv,
                             const ap_uint<l1ct::TkObj::BITWIDTH> p_track[NTRACK],
                             const ap_uint<l1ct::EGIsoObj::BITWIDTH> p_photons_in[NEM_EGOUT],
                             const ap_uint<l1ct::EGIsoEleObj::BITWIDTH> p_eles_in[NEM_EGOUT],
                             ap_uint<l1ct::EGIsoObj::BITWIDTH> p_photons_out[NEM_EGOUT],
                             ap_uint<l1ct::EGIsoEleObj::BITWIDTH> p_eles_out[NEM_EGOUT]
                           
                           ) {
  #pragma HLS ARRAY_PARTITION variable=p_track complete
  #pragma HLS ARRAY_PARTITION variable=p_photons_in complete
  #pragma HLS ARRAY_PARTITION variable=p_eles_in complete
  #pragma HLS ARRAY_PARTITION variable=p_photons_out complete
  #pragma HLS ARRAY_PARTITION variable=p_eles_out complete

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
    #pragma HLS pipeline II=2
  #endif

  l1ct::PFRegion region;
  l1ct::PVObj pv;
  l1ct::TkObj tracks[NTRACK];
  l1ct::EGIsoObj photons[NEM_EGOUT];
  l1ct::EGIsoEleObj eles[NEM_EGOUT];
  #pragma HLS ARRAY_PARTITION variable=tracks complete
  #pragma HLS ARRAY_PARTITION variable=photons complete
  #pragma HLS ARRAY_PARTITION variable=eles complete

  tkeg_tkisolation_unpack_in(p_region, p_pv, p_track, p_photons_in, p_eles_in, region, pv, tracks, photons, eles);
  tkeg_tkisolation(region, pv, tracks, photons, eles);
  tkeg_tkisolation_pack_out(photons, eles, p_photons_out, p_eles_out);
}

void tkeg_tkisolation_pack_in(const l1ct::PFRegion& region,
                              const l1ct::PVObj& pv,
                              const l1ct::TkObj track[NTRACK],
                              const l1ct::EGIsoObj photons[NEM_EGOUT],
                              const l1ct::EGIsoEleObj eles[NEM_EGOUT],
                              ap_uint<l1ct::PFRegion::BITWIDTH>& p_region,
                              ap_uint<l1ct::PVObj::BITWIDTH>& p_pv,
                              ap_uint<l1ct::TkObj::BITWIDTH> p_track[NTRACK],
                              ap_uint<l1ct::EGIsoObj::BITWIDTH> p_photons[NEM_EGOUT],
                              ap_uint<l1ct::EGIsoEleObj::BITWIDTH> p_eles[NEM_EGOUT]) {
  #pragma HLS ARRAY_PARTITION variable=track complete
  #pragma HLS ARRAY_PARTITION variable=photons complete
  #pragma HLS ARRAY_PARTITION variable=eles complete
  #pragma HLS ARRAY_PARTITION variable=p_track complete
  #pragma HLS ARRAY_PARTITION variable=p_photons complete
  #pragma HLS ARRAY_PARTITION variable=p_eles complete
  #pragma HLS inline
  #pragma HLS inline region recursive
  p_region = region.pack();
  p_pv = pv.pack();
  l1pf_pattern_pack<NTRACK>(track, p_track);
  tkeg_tkisolation_pack_out(photons, eles, p_photons, p_eles);
}

void tkeg_tkisolation_unpack_in(const ap_uint<l1ct::PFRegion::BITWIDTH>& p_region,
                                const ap_uint<l1ct::PVObj::BITWIDTH>& p_pv,
                                const ap_uint<l1ct::TkObj::BITWIDTH> p_track[NTRACK],
                                const ap_uint<l1ct::EGIsoObj::BITWIDTH> p_photons[NEM_EGOUT],
                                const ap_uint<l1ct::EGIsoEleObj::BITWIDTH> p_eles[NEM_EGOUT],
                                l1ct::PFRegion& region,
                                l1ct::PVObj& pv,
                                l1ct::TkObj track[NTRACK],
                                l1ct::EGIsoObj photons[NEM_EGOUT],
                                l1ct::EGIsoEleObj eles[NEM_EGOUT]) {
  #pragma HLS ARRAY_PARTITION variable=p_track complete
  #pragma HLS ARRAY_PARTITION variable=p_photons complete
  #pragma HLS ARRAY_PARTITION variable=p_eles complete
  #pragma HLS ARRAY_PARTITION variable=track complete
  #pragma HLS ARRAY_PARTITION variable=photons complete
  #pragma HLS ARRAY_PARTITION variable=eles complete

  #pragma HLS inline
  #pragma HLS inline region recursive
  region = PFRegion::unpack(p_region);
  pv = PVObj::unpack(p_pv);
  l1pf_pattern_unpack<NTRACK>(p_track, track);
  tkeg_tkisolation_unpack_out(p_photons, p_eles, photons, eles);                          
}

void tkeg_tkisolation_pack_out(const l1ct::EGIsoObj photons[NEM_EGOUT],
                               const l1ct::EGIsoEleObj eles[NEM_EGOUT],
                               ap_uint<l1ct::EGIsoObj::BITWIDTH> p_photons[NEM_EGOUT],
                               ap_uint<l1ct::EGIsoEleObj::BITWIDTH> p_eles[NEM_EGOUT]) {
  #pragma HLS ARRAY_PARTITION variable=photons complete
  #pragma HLS ARRAY_PARTITION variable=eles complete
  #pragma HLS ARRAY_PARTITION variable=p_photons complete
  #pragma HLS ARRAY_PARTITION variable=p_eles complete
  #pragma HLS inline
  #pragma HLS inline region recursive
  l1pf_pattern_pack<NEM_EGOUT>(photons, p_photons);
  l1pf_pattern_pack<NEM_EGOUT>(eles, p_eles);  
}

void tkeg_tkisolation_unpack_out(const ap_uint<l1ct::EGIsoObj::BITWIDTH> p_photons[NEM_EGOUT],
                                 const ap_uint<l1ct::EGIsoEleObj::BITWIDTH> p_eles[NEM_EGOUT],
                                 l1ct::EGIsoObj photons[NEM_EGOUT],
                                 l1ct::EGIsoEleObj eles[NEM_EGOUT]) {
  #pragma HLS ARRAY_PARTITION variable=photons complete
  #pragma HLS ARRAY_PARTITION variable=eles complete
  #pragma HLS ARRAY_PARTITION variable=p_photons complete
  #pragma HLS ARRAY_PARTITION variable=p_eles complete
  #pragma HLS inline
  #pragma HLS inline region recursive                        
  l1pf_pattern_unpack<NEM_EGOUT>(p_photons, photons);
  l1pf_pattern_unpack<NEM_EGOUT>(p_eles, eles);                                   
}

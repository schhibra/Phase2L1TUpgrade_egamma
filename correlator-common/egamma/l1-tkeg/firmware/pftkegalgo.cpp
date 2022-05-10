#include "pftkegalgo.h"
#include <cassert>
#include "hls_math.h"

#include "../../../pf/firmware/pfalgo_types.h"

using namespace l1ct;
#include "../../../pf/firmware/pfalgo_common.h"
#include "../../../pf/firmware/pfalgo_common.icc"


dpt_t ell_dpt_int_cap(glbeta_t abseta_ref, eta_t eta1, phi_t phi1, eta_t eta2, phi_t phi2, pt_t pt1, pt_t pt2, dpt_t max) {
#pragma HLS INLINE

#if defined(REG_HGCal)
    // const ap_uint<10> cdeta_int = 16;
    const ap_ufixed<12,10> cdeta = 16;
#endif

#if defined(REG_Barrel)
    // const ap_uint<10> cdeta_int = (hls::abs(eta1) > 3) ? 22 : 8;
    // //FIXME: global absolute eta > 0.9
    // const ap_ufixed<10,12> cdeta = (hls::abs(eta1) > 206) ? 21.75 : 7.75;
    // const ap_uint<10> cdeta = 22;
    const ap_ufixed<12,10> cdeta = 21.75;
    // const ap_ufixed<12,10> cdeta = (abseta_ref > 206) ? 21.75 : 7.75;
#endif

    const ap_uint<10> cm = 256;
    // const ap_uint<10> cm_int = 256;

    ap_int<eta_t::width+1> d_eta = (eta1-eta2);
    ap_int<phi_t::width+1> d_phi = (phi1-phi2);

    ap_uint<22> ell = d_phi*d_phi + d_eta*d_eta*cdeta;

    // std::cout << "[FW] ell: " << ell << " cm: " << cm << " match:" << (ell <= int(cm)) <<std::endl;

    dpt_t d_pt = hls::abs(pt1 - pt2);
    return (ell <= int(cm)) ? d_pt : max;
}



template<int DPTMAX>
void calo2tk_ellipticdptvals(const l1ct::PFRegion & region, const EmCaloObj &em, const TkObj track[NTRACK_EGIN], dpt_t calo_track_dptval[NTRACK_EGIN]) {
// Not inlining this allows to save DSPs with II != 1
#pragma HLS INLINE OFF
    const dpt_t eDPTMAX = DPTMAX;
    const pt_t trkQualityPtMin_ = 10.; // 10 GeV
    glbeta_t abseta = hls::abs(region.hwGlbEta(em.hwEta));
    
    track_loop: for (int itk = 0; itk < NTRACK_EGIN; ++itk) {
      if (track[itk].hwPt < trkQualityPtMin_ || em.hwPt == 0) {
        calo_track_dptval[itk] = eDPTMAX;
      } else {
        calo_track_dptval[itk] = ell_dpt_int_cap(abseta, em.hwEta, em.hwPhi, track[itk].hwEta, track[itk].hwPhi, em.hwPt, track[itk].hwPt, eDPTMAX);
        // std::cout << "[" << itk << "] dpt: " << calo_track_dptval[itk] << std::endl;
      }
    }

}



void link_emCalo2emCalo(const EmCaloObj emcalo[NEMCALO_EGIN], ap_uint<NEMCALO_EGIN> emCalo2emcalo_bit[NEMCALO_EGIN]) {
  #pragma HLS ARRAY_PARTITION variable=emcalo complete dim=1
  #pragma HLS ARRAY_PARTITION variable=emCalo2emcalo_bit complete dim=1
  #pragma HLS INLINE

  const ap_int<eta_t::width+1>  dEtaMaxBrem_ = 5; // 0.02; -> round(0.02*4*180/3.14)
  const ap_int<phi_t::width+1>  dPhiMaxBrem_ = 23; // 0.1; -> round(0.1*4*180/3.14)

  // NOTE: we assume the input to be sorted!!!
  brem_reco_outer_loop: for (int ic = 0; ic < NEMCALO_EGIN; ++ic) {
    auto &calo = emcalo[ic];
    brem_reco_inner_loop: for (int jc = ic + 1; jc < NEMCALO_EGIN; ++jc) {
      auto &otherCalo = emcalo[jc];
      ap_int<eta_t::width+1> deta =  otherCalo.hwEta - calo.hwEta;
      deta = (deta > 0) ? deta : ap_int<eta_t::width+1>(-deta);
      ap_int<phi_t::width+1> dphi = otherCalo.hwPhi - calo.hwPhi;
      dphi = (dphi > 0) ? dphi : ap_int<phi_t::width+1>(-dphi);
      if (calo.hwPt != 0 && otherCalo.hwPt != 0 &&
        deta < dEtaMaxBrem_ &&
        dphi < dPhiMaxBrem_) {
            emCalo2emcalo_bit[ic][jc] = 1;
            emCalo2emcalo_bit[jc][jc] = 1; // use diagonal bit to mark the cluster as already used
            // std::cout << "[FW] BREM: set to be used " << ic << " " << jc << std::endl;
            // std::cout << "[FW] BREM: set to skip " << jc << " " << jc << std::endl;
      }
    }
  }
}





void link_emCalo2tk(const l1ct::PFRegion & region,
                    const EmCaloObj emcalo[NEMCALO_EGIN],
                    const TkObj track[NTRACK_EGIN],
                    ap_uint<NTRACK_EGIN> emCalo2tk_bit[NEMCALO_EGIN]) {
  #pragma HLS INLINE

  #pragma HLS ARRAY_PARTITION variable=emcalo complete dim=1
  #pragma HLS ARRAY_PARTITION variable=track complete dim=1
  #pragma HLS ARRAY_PARTITION variable=emCalo2tk_bit complete dim=1

  const int DPTMAX = 16384; // dpt_t = ap_fixed<16,14,AP_TRN,AP_SAT> ->  max =(2^(16) - 1)/2^2 = 16383.75
  calo_loop: for (int ic = 0; ic < NEMCALO_EGIN; ++ic) {
    dpt_t dptvals[NTRACK_EGIN];
    calo2tk_ellipticdptvals<DPTMAX>(region, emcalo[ic], track, dptvals);
    emCalo2tk_bit[ic] = pick_closest<DPTMAX,NTRACK_EGIN,dpt_t>(dptvals);
  }

}

void pftkegalgo(const l1ct::PFRegion & region, 
                const EmCaloObj emcalo[NEMCALO_EGIN], 
                const TkObj track[NTRACK_EGIN],
                EGIsoObj photons[NEM_EGOUT], 
                EGIsoEleObj eles[NEM_EGOUT]) {
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
  // #pragma HLS PIPELINE II=HLS_pipeline_II
  #pragma HLS ARRAY_PARTITION variable=emcalo complete dim=1
  #pragma HLS ARRAY_PARTITION variable=track complete dim=1
  #pragma HLS ARRAY_PARTITION variable=photons complete dim=1
  #pragma HLS ARRAY_PARTITION variable=eles complete dim=1

  ap_uint<NTRACK_EGIN> emCalo2tk_bit[NEMCALO_EGIN];
  ap_uint<NEMCALO_EGIN> emCalo2emcalo_bit[NEMCALO_EGIN];
  #pragma HLS ARRAY_PARTITION variable=emCalo2tk_bit complete dim=1
  #pragma HLS ARRAY_PARTITION variable=emCalo2emcalo_bit complete dim=1

  #pragma HLS INLINE REGION recursive
  // initialize
  init_loop: for (int ic = 0; ic < NEMCALO_EGIN; ++ic) {
    emCalo2tk_bit[ic] = 0;
    emCalo2emcalo_bit[ic] = 0;
  }

  #if defined(DOBREMRECOVERY)
    link_emCalo2emCalo(emcalo, emCalo2emcalo_bit);
  #endif
  //
  link_emCalo2tk(region, emcalo, track, emCalo2tk_bit);


  EGIsoObj photons_temp[NEMCALO_EGIN];
  EGIsoEleObj eles_temp[NEMCALO_EGIN];
  #pragma HLS ARRAY_PARTITION variable=photons_temp complete dim=1
  #pragma HLS ARRAY_PARTITION variable=eles_temp complete dim=1

  loop_calo: for (int ic = 0; ic < NEMCALO_EGIN; ++ic) {

    // if(emcalo_sel[ic].hwPt > 0)std::cout << "[FW] emcalo [" << ic << "]  with pt: " << emcalo_sel[ic].hwPt << " qual: " << emcalo_sel[ic].hwEmID << " eta: " << emcalo_sel[ic].hwEta << " phi " << emcalo_sel[ic].hwPhi << std::endl;

    clear(photons_temp[ic]);
    clear(eles_temp[ic]);
    
    if(emcalo[ic].hwPt == 0) continue;

    int track_id = -1;
    loop_track_matched: for(int it = 0; it < NTRACK_EGIN; ++it) {
      if(emCalo2tk_bit[ic][it]) {
        track_id = it;
        break;
      }
    }

    pt_t ptcorr = emcalo[ic].hwPt;
    egquality_t hwQual = egquality_t(emcalo[ic].hwEmID);
    #if defined(DOBREMRECOVERY)
      if(emCalo2emcalo_bit[ic][ic] != 1) {
        hwQual++;
        // FIXME: we should set the quality bit as "brem-recovery performed"
        loop_calo_brem_reco: for (int ioc = 0; ioc < NEMCALO_EGIN; ++ioc) {
          if(emCalo2emcalo_bit[ic][ioc]) {
            ptcorr += emcalo[ioc].hwPt;
          }
        }
      } else {
        // This cluster has alread been used in brem reclustering
        // shall we just set the quality to a different value???
        // std::cout << "   skip!" << std::endl;
        continue;
      }
    #endif

    photons_temp[ic].hwPt = ptcorr;
    photons_temp[ic].hwEta = emcalo[ic].hwEta;
    photons_temp[ic].hwPhi = emcalo[ic].hwPhi;
    photons_temp[ic].hwQual = hwQual;
    // if(photons_temp[ic].hwPt) std::cout << "[FW] Add EGIsoObj with pt: " << ptcorr << " qual: " << photons_temp[ic].hwQual << " eta: " << photons_temp[ic].hwEta << " phi " << photons_temp[ic].hwPhi << std::endl;

    if(emCalo2tk_bit[ic]) {
      eles_temp[ic].hwPt = ptcorr;
      eles_temp[ic].hwEta = emcalo[ic].hwEta;
      eles_temp[ic].hwPhi = emcalo[ic].hwPhi;
      eles_temp[ic].hwQual = hwQual;
      eles_temp[ic].hwDEta = track[track_id].hwVtxEta() - eles_temp[ic].hwEta;
      eles_temp[ic].hwDPhi = hls::abs(track[track_id].hwVtxPhi() - eles_temp[ic].hwPhi);
      eles_temp[ic].hwCharge = track[track_id].hwCharge;
      eles_temp[ic].hwZ0 = track[track_id].hwZ0;
      // if(eles_temp[ic].hwPt) std::cout << "[FW] Add EGIsoEleObj with pt: " << ptcorr << " qual: " <<   eles_temp[ic].hwQual << " eta: " << eles_temp[ic].hwEta << " phi " << eles_temp[ic].hwPhi << std::endl;

    }

  }
  ptsort_hwopt<EGIsoObj,NEMCALO_EGIN,NEM_EGOUT>(photons_temp, photons);
  ptsort_hwopt<EGIsoEleObj,NEMCALO_EGIN,NEM_EGOUT>(eles_temp, eles);

}

void pftkegalgo_pack_in(const l1ct::PFRegion & region,
                        const l1ct::EmCaloObj emcalo[NEMCALO],
                        const l1ct::TkObj track[NTRACK],
                        ap_uint<l1ct::PFRegion::BITWIDTH> & packed_region,
                        ap_uint<l1ct::EmCaloObj::BITWIDTH> packed_emcalo[NEMCALO],
                        ap_uint<l1ct::TkObj::BITWIDTH> packed_track[NTRACK]) {
  #pragma HLS ARRAY_PARTITION variable=emcalo complete
  #pragma HLS ARRAY_PARTITION variable=track complete
  #pragma HLS ARRAY_PARTITION variable=packed_emcalo complete
  #pragma HLS ARRAY_PARTITION variable=packed_track complete
  #pragma HLS inline
  #pragma HLS inline region recursive
  packed_region = region.pack();
  l1pf_pattern_pack<NEMCALO>(emcalo, packed_emcalo);
  l1pf_pattern_pack<NTRACK>(track, packed_track);
}

void pftkegalgo_unpack_in(const ap_uint<l1ct::PFRegion::BITWIDTH> & packed_region,
                          const ap_uint<l1ct::EmCaloObj::BITWIDTH> packed_emcalo[NEMCALO],
                          const ap_uint<l1ct::TkObj::BITWIDTH> packed_track[NTRACK],
                          l1ct::PFRegion & region,
                          l1ct::EmCaloObj emcalo[NEMCALO],
                          l1ct::TkObj track[NTRACK]) {
  #pragma HLS ARRAY_PARTITION variable=emcalo complete
  #pragma HLS ARRAY_PARTITION variable=track complete
  #pragma HLS ARRAY_PARTITION variable=packed_emcalo complete
  #pragma HLS ARRAY_PARTITION variable=packed_track complete
  #pragma HLS inline
  #pragma HLS inline region recursive
  region = PFRegion::unpack(packed_region);
  l1pf_pattern_unpack<NEMCALO>(packed_emcalo, emcalo);
  l1pf_pattern_unpack<NTRACK>(packed_track, track);
}

void pftkegalgo_pack_out(const l1ct::EGIsoObj photons[NEM_EGOUT],
                         const l1ct::EGIsoEleObj eles[NEM_EGOUT],
                         ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_photons[NEM_EGOUT],
                         ap_uint<l1ct::EGIsoEleObj::BITWIDTH> packed_eles[NEM_EGOUT]) {
   #pragma HLS ARRAY_PARTITION variable=photons complete
   #pragma HLS ARRAY_PARTITION variable=eles complete
   #pragma HLS ARRAY_PARTITION variable=packed_photons complete
   #pragma HLS ARRAY_PARTITION variable=packed_eles complete
   #pragma HLS inline
   #pragma HLS inline region recursive
   l1pf_pattern_pack<NEM_EGOUT>(photons, packed_photons);
   l1pf_pattern_pack<NEM_EGOUT>(eles, packed_eles);
}

void pftkegalgo_unpack_out(const ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_photons[NEM_EGOUT],
                           const ap_uint<l1ct::EGIsoEleObj::BITWIDTH> packed_eles[NEM_EGOUT],
                           l1ct::EGIsoObj photons[NEM_EGOUT],
                           l1ct::EGIsoEleObj eles[NEM_EGOUT]) {
   #pragma HLS ARRAY_PARTITION variable=photons complete
   #pragma HLS ARRAY_PARTITION variable=eles complete
   #pragma HLS ARRAY_PARTITION variable=packed_photons complete
   #pragma HLS ARRAY_PARTITION variable=packed_eles complete
   #pragma HLS inline
   #pragma HLS inline region recursive                        
   l1pf_pattern_unpack<NEM_EGOUT>(packed_photons, photons);
   l1pf_pattern_unpack<NEM_EGOUT>(packed_eles, eles);
}


void packed_pftkegalgo(const ap_uint<l1ct::PFRegion::BITWIDTH> & packed_region,
                       const ap_uint<l1ct::EmCaloObj::BITWIDTH> packed_emcalo[NEMCALO],
                       const ap_uint<l1ct::TkObj::BITWIDTH> packed_track[NTRACK],
                       ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_photons[NEM_EGOUT],
                       ap_uint<l1ct::EGIsoEleObj::BITWIDTH> packed_eles[NEM_EGOUT]) {
  #pragma HLS ARRAY_PARTITION variable=packed_emcalo complete
  #pragma HLS ARRAY_PARTITION variable=packed_track complete
  #pragma HLS ARRAY_PARTITION variable=packed_photons complete
  #pragma HLS ARRAY_PARTITION variable=packed_eles complete

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
  l1ct::EmCaloObj emcalo[NEMCALO];
  l1ct::TkObj track[NTRACK];
  l1ct::EGIsoObj photons[NEM_EGOUT];
  l1ct::EGIsoEleObj eles[NEM_EGOUT];
  #pragma HLS ARRAY_PARTITION variable=emcalo complete dim=1
  #pragma HLS ARRAY_PARTITION variable=track complete dim=1
  #pragma HLS ARRAY_PARTITION variable=photons complete dim=1
  #pragma HLS ARRAY_PARTITION variable=eles complete dim=1

  pftkegalgo_unpack_in(packed_region, packed_emcalo, packed_track, region, emcalo, track);
  pftkegalgo(region, emcalo, track, photons, eles);
  pftkegalgo_pack_out(photons, eles, packed_photons, packed_eles);
}

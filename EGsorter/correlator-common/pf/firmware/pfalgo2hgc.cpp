#include "pfalgo2hgc.h"
#include <cassert>

using namespace l1ct;

#include "pfalgo_common.icc"


void tk2calo_sumtk_hgc(const TkObj track[NTRACK], const bool isMu[NTRACK], const pt2_t tkerr2[NTRACK], const ap_uint<NCALO> calo_track_link_bit[NTRACK], pt_t sumtk[NCALO], pt2_t sumtkerr2[NCALO]) {
    for (int icalo = 0; icalo < NCALO; ++icalo) {
        pt_t sum = 0;
        pt2_t sumerr = 0;
        for (int it = 0; it < NTRACK; ++it) {
            if (calo_track_link_bit[it][icalo] && !isMu[it]) { sum += track[it].hwPt; sumerr += tkerr2[it]; }
        }
        sumtk[icalo] = sum;
        sumtkerr2[icalo] = sumerr;
    }
}

void tk2calo_elealgo_hgc(const TkObj track[NTRACK], const HadCaloObj calo[NCALO], const ap_uint<NCALO> calo_track_link_bit[NTRACK], bool isEle[NTRACK]) {
    for (int it = 0; it < NTRACK; ++it) {
        bool ele = false;
        for (int icalo = 0; icalo < NCALO; ++icalo) {
            if (calo_track_link_bit[it][icalo] && calo[icalo].hwIsEM()) ele = true;
        }
        isEle[it] = ele;
    }
} 

void tk2calo_tkalgo_hgc(const TkObj track[NTRACK], const bool isEle[NTRACK], const bool isMu[NTRACK], const ap_uint<NCALO> calo_track_link_bit[NTRACK], PFChargedObj pfout[NTRACK]) {
    const pt_t TKPT_MAX_LOOSE = PFALGO_TK_MAXINVPT_LOOSE;
    const pt_t TKPT_MAX_TIGHT = PFALGO_TK_MAXINVPT_TIGHT;
    for (int it = 0; it < NTRACK; ++it) {
        bool goodByPt = track[it].hwPt < (track[it].isPFTight() ? TKPT_MAX_TIGHT : TKPT_MAX_LOOSE);
        bool good = isMu[it] || isEle[it] || goodByPt || calo_track_link_bit[it].or_reduce();
        bool nonnull = track[it].isPFLoose() && track[it].hwPt != 0;
        if (nonnull && good) {
            pfout[it].hwPt  = track[it].hwPt;
            pfout[it].hwEta = track[it].hwEta;
            pfout[it].hwPhi = track[it].hwPhi;
            pfout[it].hwId  = isMu[it] ? ParticleID::mkMuon(track[it].hwCharge) : ( isEle[it] ? ParticleID::mkElectron(track[it].hwCharge) : ParticleID::mkChHad(track[it].hwCharge) );
            pfout[it].hwDEta = track[it].hwDEta;
            pfout[it].hwDPhi = track[it].hwDPhi;
            pfout[it].hwZ0  = track[it].hwZ0;
            pfout[it].hwDxy = track[it].hwDxy;
            pfout[it].hwTkQuality = track[it].hwQuality;
        } else {
            clear(pfout[it]);
        }
    }
}


void tk2calo_caloalgo_hgc(const HadCaloObj calo[NCALO], const pt_t sumtk[NCALO], const pt2_t sumtkerr2[NCALO], PFNeutralObj pfout[NCALO]) {
    for (int icalo = 0; icalo < NCALO; ++icalo) {
        pt_t calopt;
        if (sumtk[icalo] == 0) {
            calopt = calo[icalo].hwPt;
        } else {
            pt_t ptdiff = calo[icalo].hwPt - sumtk[icalo];
            pt2_t ptdiff2 = ptdiff*ptdiff;
#ifdef L1PF_DSP_LATENCY3
            #pragma HLS resource variable=ptdiff2 latency=3
#endif
            if (ptdiff > 0 && (ptdiff2 > sumtkerr2[icalo])) {
                calopt = ptdiff;
            } else {
                calopt = 0;
            }
        }
        pfout[icalo].hwPt  = calopt;
        pfout[icalo].hwEta = calopt ? calo[icalo].hwEta : eta_t(0);
        pfout[icalo].hwPhi = calopt ? calo[icalo].hwPhi : phi_t(0);
        pfout[icalo].hwId  = ParticleID(calopt ? (calo[icalo].hwIsEM() ? ParticleID::PHOTON : ParticleID::HADZERO) : ParticleID::NONE);
        pfout[icalo].hwEmPt  = calo[icalo].hwIsEM() ? calopt : pt_t(0); // FIXME
        pfout[icalo].hwEmID  = calopt ? calo[icalo].hwEmID : emid_t(0);
        pfout[icalo].hwPUID  = 0;
    }
}



void pfalgo2hgc(const PFRegion & region, const HadCaloObj calo[NCALO], const TkObj track[NTRACK], const MuObj mu[NMU], PFChargedObj outch[NTRACK], PFNeutralObj outne[NSELCALO], PFChargedObj outmu[NMU]) {
    #pragma HLS ARRAY_PARTITION variable=calo complete
    #pragma HLS ARRAY_PARTITION variable=track complete
    #pragma HLS ARRAY_PARTITION variable=mu complete    
    #pragma HLS ARRAY_PARTITION variable=outch complete
    #pragma HLS ARRAY_PARTITION variable=outne complete
    #pragma HLS ARRAY_PARTITION variable=outmu complete

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

    // ---------------------------------------------------------------

    // ---------------------------------------------------------------
    // TK-MU Linking
    ap_uint<NMU> mu_track_link_bit[NTRACK];
    #pragma HLS ARRAY_PARTITION variable=mu_track_link_bit complete
    bool isMu[NTRACK];
    for (int it = 0; it < NTRACK; ++it) { isMu[it] = 0; }

    mutrk_link(mu, track, mu_track_link_bit);
    pfmualgo(mu, track, mu_track_link_bit, outmu, isMu);

    bool isEle[NTRACK];
    #pragma HLS ARRAY_PARTITION variable=isEle complete

    // ---------------------------------------------------------------
    // TK-HAD Linking
    
    pt_t tkerr[NTRACK];
    #pragma HLS ARRAY_PARTITION variable=tkerr complete
    tk2calo_tkerr(region, track, tkerr);

    ap_uint<NCALO> calo_track_link_bit[NTRACK];
    #pragma HLS ARRAY_PARTITION variable=calo_track_link_bit complete
    tk2calo_link_drdpt(calo, track, tkerr, calo_track_link_bit);
    //tk2calo_link_dronly(hadcalo_sub, track, calo_track_link_bit);

    pt2_t tkerr2[NTRACK];
    #pragma HLS ARRAY_PARTITION variable=tkerr2 complete
    tk2calo_tkerr2(tkerr, tkerr2);

    pt_t sumtk[NCALO]; pt2_t sumtkerr2[NCALO];
    #pragma HLS ARRAY_PARTITION variable=sumtk complete
    #pragma HLS ARRAY_PARTITION variable=sumtkerr2 complete

    tk2calo_elealgo_hgc(track, calo, calo_track_link_bit, isEle);

    tk2calo_tkalgo_hgc(track, isEle, isMu, calo_track_link_bit, outch);
    tk2calo_sumtk_hgc(track,  isMu, tkerr2, calo_track_link_bit, sumtk, sumtkerr2);

#if NCALO == NSELCALO
    tk2calo_caloalgo_hgc(calo, sumtk, sumtkerr2, outne);
#else
    PFNeutralObj outne_all[NCALO];
    #pragma HLS ARRAY_PARTITION variable=outne_all complete
    tk2calo_caloalgo_hgc(calo, sumtk, sumtkerr2, outne_all);
    ptsort_hwopt<PFNeutralObj,NCALO,NSELCALO>(outne_all, outne);
#endif

}

void packed_pfalgo2hgc(const ap_uint<PFALGO2HGC_DATA_SIZE> input[PFALGO2HGC_NCHANN_IN], ap_uint<PFALGO2HGC_DATA_SIZE> output[PFALGO2HGC_NCHANN_OUT]) {
    #pragma HLS ARRAY_PARTITION variable=input complete
    #pragma HLS ARRAY_PARTITION variable=output complete
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

    // ---------------------------------------------------------------
    PFRegion region; 
    HadCaloObj calo[NCALO]; TkObj track[NTRACK]; MuObj mu[NMU]; 
    PFChargedObj outch[NTRACK]; PFNeutralObj outne[NSELCALO]; PFChargedObj outmu[NMU];
    #pragma HLS ARRAY_PARTITION variable=calo complete
    #pragma HLS ARRAY_PARTITION variable=track complete
    #pragma HLS ARRAY_PARTITION variable=mu complete    
    #pragma HLS ARRAY_PARTITION variable=outch complete
    #pragma HLS ARRAY_PARTITION variable=outne complete
    #pragma HLS ARRAY_PARTITION variable=outmu complete

    pfalgo2hgc_unpack_in(input, region, calo, track, mu);
    pfalgo2hgc(region, calo, track, mu, outch, outne, outmu);
    pfalgo2hgc_pack_out(outch, outne, outmu, output);
}

void pfalgo2hgc_pack_in(const PFRegion & region, const HadCaloObj calo[NCALO], const TkObj track[NTRACK], const MuObj mu[NMU], ap_uint<PFALGO2HGC_DATA_SIZE> input[PFALGO2HGC_NCHANN_IN]) {
    const unsigned int TRACK_OFFS = 1, CALO_OFFS = TRACK_OFFS + NTRACK, MU_OFFS = CALO_OFFS + NCALO;
    input[0] = region.pack();
    l1pf_pattern_pack<NTRACK,TRACK_OFFS>(track, input);
    l1pf_pattern_pack<NCALO, CALO_OFFS >(calo,  input);
    l1pf_pattern_pack<NMU,   MU_OFFS   >(mu,    input);
}

void pfalgo2hgc_unpack_in(const ap_uint<PFALGO2HGC_DATA_SIZE> input[PFALGO2HGC_NCHANN_IN], PFRegion & region, HadCaloObj calo[NCALO], TkObj track[NTRACK], MuObj mu[NMU]) {
    #pragma HLS ARRAY_PARTITION variable=input complete
    #pragma HLS ARRAY_PARTITION variable=calo complete
    #pragma HLS ARRAY_PARTITION variable=track complete
    #pragma HLS ARRAY_PARTITION variable=mu complete    
    #pragma HLS inline 
    #pragma HLS inline region recursive
    const unsigned int TRACK_OFFS = 1, CALO_OFFS = TRACK_OFFS + NTRACK, MU_OFFS = CALO_OFFS + NCALO;
    region = PFRegion::unpack(input[0]);
    l1pf_pattern_unpack<NTRACK,TRACK_OFFS>(input, track);
    l1pf_pattern_unpack<NCALO, CALO_OFFS >(input, calo);
    l1pf_pattern_unpack<NMU,   MU_OFFS   >(input, mu);
}

void pfalgo2hgc_pack_out(const PFChargedObj outch[NTRACK], const PFNeutralObj outne[NSELCALO], const PFChargedObj outmu[NMU], ap_uint<PFALGO2HGC_DATA_SIZE> output[PFALGO2HGC_NCHANN_OUT]) {
    #pragma HLS ARRAY_PARTITION variable=output complete
    #pragma HLS ARRAY_PARTITION variable=outch complete
    #pragma HLS ARRAY_PARTITION variable=outne complete
    #pragma HLS ARRAY_PARTITION variable=outmu complete
    #pragma HLS inline
    #pragma HLS inline region recursive

    const int PFCH_OFFS = 0, PFNE_OFFS = PFCH_OFFS + NTRACK, PFMU_OFFS = PFNE_OFFS + NSELCALO;
    l1pf_pattern_pack<NTRACK,  PFCH_OFFS>(outch, output);
    l1pf_pattern_pack<NSELCALO,PFNE_OFFS>(outne, output);
    l1pf_pattern_pack<NMU,     PFMU_OFFS>(outmu, output);
}

void pfalgo2hgc_unpack_out(const ap_uint<PFALGO2HGC_DATA_SIZE> output[PFALGO2HGC_NCHANN_OUT], PFChargedObj outch[NTRACK], PFNeutralObj outne[NSELCALO], PFChargedObj outmu[NMU]) {
    const int PFCH_OFFS = 0, PFNE_OFFS = PFCH_OFFS + NTRACK, PFMU_OFFS = PFNE_OFFS + NSELCALO;
    l1pf_pattern_unpack<NTRACK,  PFCH_OFFS>(output, outch);
    l1pf_pattern_unpack<NSELCALO,PFNE_OFFS>(output, outne);
    l1pf_pattern_unpack<NMU,     PFMU_OFFS>(output, outmu);
}


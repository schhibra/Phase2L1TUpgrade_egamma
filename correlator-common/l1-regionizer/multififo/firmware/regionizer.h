#ifndef multififo_regionizer_h
#define multififo_regionizer_h

#include "../../../dataformats/layer1_objs.h"
#include "../../../dataformats/layer1_multiplicities.h"

typedef ap_uint<72> PackedTkObj;
typedef ap_uint<72> PackedCaloObj;
typedef ap_uint<72> PackedMuObj;
inline void clear(ap_uint<72> & o) { o = 0; }

template<typename P>
inline P phiShifted(const P & t, int phi_shift) {
    #pragma HLS inline
    P ret = t;
    ret.hwPhi += l1ct::phi_t(phi_shift);
    return ret;
}
template<typename P>
inline P etaShifted(const P & t, l1ct::eta_t eta_shift) {
    #pragma HLS inline
    P ret = t;
    ret.hwEta += eta_shift;
    return ret;
}
template<typename P>
inline P etaPhiShifted(const P & t, l1ct::eta_t eta_shift, int phi_shift) {
    #pragma HLS inline
    P ret = t;
    ret.hwEta += eta_shift;
    ret.hwPhi += l1ct::phi_t(phi_shift);
    return ret;
}




#define REGIONIZERNCLOCKS 54
#define NPFREGIONS 9
#define PFREGION_PHI_SIZE 160  // size of a phi sector (in L1PF units, LSB = 0.25 degrees)
#define PFREGION_PHI_BORDER 57 // size of the phi border of a PF region (0.25 rad = 57, 0.30 rad = 69)
#define PFREGION_ETA_SIZE 230  // size of an eta sector: 1.0 rad => 229, round up to 230 be even  
#define PFREGION_ETA_BORDER 57 // size of the eta border of a PF region (0.25 rad = 57, 0.30 rad = 69)
#if defined(ROUTER_NOSTREAM)
#define PFLOWII  6
#else
#define PFLOWII  4
#endif

#define NTKSECTORS 9
#define NTKFIBERS  2
#define NTKFIFOS   6
#define NTKSORTED  NTRACK
#define NTKSTREAMS ((NTKSORTED+PFLOWII-1)/PFLOWII)

#define NCALOSECTORS 3
#define NCALOFIBERS  4 
#define NCALOFIFOS0 NCALOFIBERS
#define NCALOFIFOS12 (2*NCALOFIBERS)
#define NCALOFIFOS (NCALOFIFOS0+2*NCALOFIFOS12)
#define NCALOSORTED  NCALO
#define NCALOSTREAMS ((NCALOSORTED+PFLOWII-1)/PFLOWII)
#define NEMSORTED  NEMCALO
#define NEMSTREAMS ((NEMSORTED+PFLOWII-1)/PFLOWII)

#define NMUFIBERS  2
#define NMUSORTED  NMU
#define NMUSTREAMS ((NMUSORTED+PFLOWII-1)/PFLOWII)

#if defined(ROUTER_NOMERGE)
    #define NTKOUT (NTKSECTORS*NTKFIFOS)
    #define NCALOOUT (NCALOSECTORS*NCALOFIFOS)
    #define NEMOUT (NCALOSECTORS*NCALOFIFOS)
    #define NMUOUT NPFREGIONS
#elif defined(ROUTER_NOMUX)
    #define NTKOUT NPFREGIONS
    #define NCALOOUT NPFREGIONS
    #define NEMOUT NPFREGIONS
    #define NMUOUT NPFREGIONS
#elif defined(ROUTER_NOSTREAM)
    #define NTKOUT NTKSORTED
    #define NCALOOUT NCALOSORTED
    #define NEMOUT NEMSORTED
    #define NMUOUT NMUSORTED
#else
    #define NTKOUT NTKSTREAMS
    #define NCALOOUT NCALOSTREAMS
    #define NEMOUT NEMSTREAMS
    #define NMUOUT NMUSTREAMS
#endif

bool tk_router(bool newevent, const PackedTkObj tracks_in[NTKSECTORS][NTKFIBERS], PackedTkObj tracks_out[NTKOUT], bool & newevent_out);
bool calo_router(bool newevent, const PackedCaloObj calo_in[NCALOSECTORS][NCALOFIBERS], PackedCaloObj calo_out[NCALOOUT], bool & newevent_out);
bool mu_router(bool newevent, const l1ct::glbeta_t etaCenter, const PackedMuObj mu_in[NMUFIBERS], PackedMuObj mu_out[NMUOUT], bool & newevent_out);

#ifndef __SYNTHESIS__
#include <cstdio>

inline void printTrack(FILE *f, const l1ct::TkObj & t) { 
    fprintf(f,"%3d % 4d % 4d %4d %4d %1d ", t.intPt(), t.intEta(), t.intPhi(), t.hwDEta.to_int(), t.hwDPhi.to_int(), int(t.hwCharge)); // note no leading +'s or 0's, they confuse VHDL text parser
}
inline void printCalo(FILE *f, const l1ct::HadCaloObj & t) { 
    fprintf(f,"%3d % 4d % 4d  ", t.intPt(), t.intEta(), t.intPhi()); // note no leading +'s or 0's, they confuse VHDL text parser
}
inline void printMu(FILE *f, const l1ct::MuObj & t) { 
    fprintf(f,"%3d % 4d % 4d  ", t.intPt(), t.intEta(), t.intPhi()); // note no leading +'s or 0's, they confuse VHDL text parser
}

inline void printTrackShort(FILE *f, const l1ct::TkObj & t) { 
   fprintf(f,"%3d%+04d ", t.intPt(), t.intPhi());
}
inline void printCaloShort(FILE *f, const l1ct::HadCaloObj & t) { 
    fprintf(f,"%3d%+04d ", t.intPt(), t.intPhi());
}
#endif



#endif


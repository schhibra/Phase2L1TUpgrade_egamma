#ifndef FIRMWARE_dataformats_layer1_multiplicities_h
#define FIRMWARE_dataformats_layer1_multiplicities_h

// DEFINE MULTIPLICITIES
#if defined(REG_HGCal)
#define NTRACK 30
#define NCALO 20
#define NMU 4
#define NSELCALO 20
#define NALLNEUTRALS NSELCALO
#define NPUPPIFINALSORTED 18
// dummy
// FIXME: select upstream and tune this number
#define NEMCALO 10
#define NPHOTON NEMCALO
// not used but must be there because used in header files
#define NNEUTRALS 1
// Configuration of EG algo follows
#define NEMCALO_EGIN 10
#define NTRACK_EGIN 10

#define DOBREMRECOVERY
#define NEM_EGOUT 5

//--------------------------------
#elif defined(REG_HGCalNoTK)
#define NCALO 12
#define NNEUTRALS 8
#define NALLNEUTRALS NCALO
#define NPUPPIFINALSORTED NALLNEUTRALS
// dummy
#define NMU 1
#define NTRACK 1
#define NEMCALO 1
#define NPHOTON NEMCALO
#define NSELCALO 1
//--------------------------------
#elif defined(REG_HF)
#define NCALO 18
#define NNEUTRALS 10
#define NALLNEUTRALS NCALO
#define NPUPPIFINALSORTED NNEUTRALS 
// dummy
#define NMU 1
#define NTRACK 1
#define NEMCALO 1
#define NPHOTON NEMCALO
#define NSELCALO 1
//--------------------------------
#else // BARREL
#ifndef REG_Barrel
#ifndef CMSSW_GIT_HASH
#warning                                                                       \
    "No region defined, assuming it's barrel (#define REG_Barrel to suppress this)"
#endif
#endif
#if defined(BOARD_KU15P)
#define NTRACK 14
#define NCALO 110
#define NMU 2
#define NEMCALO 10
#define NPHOTON NEMCALO
#define NSELCALO 10
#define NALLNEUTRALS (NPHOTON + NSELCALO)
#define NNEUTRALS 15
#define NPUPPIFINALSORTED NNEUTRALS 
// Configuration of EG algo follows
#define NEMCALO_EGIN 10
#define NTRACK_EGIN 13
#define NEM_EGOUT 10
#elif defined(BOARD_VCU118)
#define NTRACK 22
#define NCALO 15
#define NEMCALO 13
#define NMU 2
#define NPHOTON NEMCALO
#define NSELCALO 10
#define NALLNEUTRALS (NPHOTON + NSELCALO)
#define NNEUTRALS 25
#define NPUPPIFINALSORTED 18
// Configuration of EG algo follows
#define NEMCALO_EGIN 10
#define NTRACK_EGIN 13
#define NEM_EGOUT 10
#else
#define NTRACK 22
#define NCALO 15
#define NEMCALO 13
#define NMU 2
#define NPHOTON NEMCALO
#define NSELCALO 13
#define NALLNEUTRALS (NPHOTON + NSELCALO)
#define NNEUTRALS 25
#define NPUPPIFINALSORTED 18
// Configuration of EG algo follows
#define NEMCALO_EGIN 10
#define NTRACK_EGIN 13
#define NEM_EGOUT 10
#endif

#endif // region

#if defined(BOARD_KU15P)
#define PACKING_DATA_SIZE 64
#define PACKING_NCHANN 42
#elif defined(BOARD_VCU118)
#define PACKING_DATA_SIZE 64
#define PACKING_NCHANN 120
#elif defined(BOARD_APD1)
#define PACKING_DATA_SIZE 64
#define PACKING_NCHANN 96
#else
#define PACKING_DATA_SIZE 64
#endif


namespace l1ct {

    template <int N> struct ct_log2_ceil {
      enum { value = ct_log2_ceil<(N / 2) + (N % 2)>::value + 1 };
    };
    template <> struct ct_log2_ceil<2> {
      enum { value = 1 };
    };
    template <> struct ct_log2_ceil<1> {
      enum { value = 0 };
    };

} // namespace


#endif

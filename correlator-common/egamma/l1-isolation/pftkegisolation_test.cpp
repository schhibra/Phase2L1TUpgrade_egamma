#include <cstdio>
#include "firmware/pftkegisolation.h"

#include "../l1-tkeg/ref/pftkegalgo_ref.h"

#include "../../utils/DumpFileReader.h"
#include "../../utils/pattern_serializer.h"
#include "../../utils/test_utils.h"

#define NTEST 10000

using namespace l1ct;

int main() {
  HumanReadablePatternSerializer debugHR("-",
                                         /*zerosuppress=*/true);  // this will print on stdout, we'll use it for errors

#if defined(REG_HGCal)
  std::cout << "REG_Hgcal" << std::endl;
  DumpFileReader inputs("TTbar_PU200_HGCal.dump");
  bool filterHwQuality = false;
  bool doBremRecovery = true;
#endif

#if defined(REG_Barrel)
  std::cout << "REG_Barrel" << std::endl;
  DumpFileReader inputs("TTbar_PU200_Barrel.dump");
  bool filterHwQuality = false;
  bool doBremRecovery = false;
#endif

  bool doTkIso = true;
  bool doPfIso = false;

  PFTkEGAlgoEmuConfig cfg(NTRACK, NTRACK_EGIN, NEMCALO_EGIN, NEM_EGOUT, filterHwQuality, doBremRecovery);
  cfg.tkIsoParams_tkEle = {2., 0.6, 0.03, 0.2};
  cfg.tkIsoParams_tkEm = {2., 0.6, 0.07, 0.3};
  cfg.pfIsoParams_tkEle = {1., 0.6, 0.03, 0.2};
  cfg.pfIsoParams_tkEm = {1., 0.6, 0.07, 0.3};
  cfg.doTkIso = doTkIso;
  cfg.doPfIso = doPfIso;
  cfg.hwIsoTypeTkEle = EGIsoEleObjEmu::IsoType::TkIso;
  cfg.hwIsoTypeTkEm = EGIsoObjEmu::IsoType::TkIsoPV;
  cfg.emClusterPtMin = 0.;
  
  l1ct::PFTkEGAlgoEmulator emulator(cfg);

  // input TP objects
  PFRegion region;
  TkObj track[NTRACK];
  PVObj pv;

  // z0_t hwZPV;
  // MuObj mu[NMU];

  // output PF objects
  l1ct::OutputRegion out_ref;  // emulator
  EGIsoObj egphs_ref[NEM_EGOUT];
  EGIsoEleObj egele_ref[NEM_EGOUT];
  EGIsoObj egphs[NEM_EGOUT];
  EGIsoEleObj egele[NEM_EGOUT];

#ifndef BOARD_none
  ap_uint<l1ct::PFRegion::BITWIDTH> packed_region;
  ap_uint<l1ct::PVObj::BITWIDTH> packed_pv;
  ap_uint<l1ct::TkObj::BITWIDTH> packed_track[NTRACK];
  ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_photons[NEM_EGOUT];
  ap_uint<l1ct::EGIsoEleObj::BITWIDTH> packed_eles[NEM_EGOUT];
  ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_photons_out[NEM_EGOUT];
  ap_uint<l1ct::EGIsoEleObj::BITWIDTH> packed_eles_out[NEM_EGOUT];

#endif

  // -----------------------------------------
  // run multiple tests
  int debug = 0;

  int passed_tests = 0;
  int nEles_tests = 0;
  int nPhotons_tests = 0;

  int tot_errors = 0;
  for (int test = 0; test < NTEST; ++test) {
    if (debug)
      std::cout << "------------ TEST: " << test << std::endl;
    // get the inputs from the input object
    if (!inputs.nextPFRegion())
      break;
    //FIXME: might need to read per event to loop on input and output regions to get the PF cands

    if (test < 1)
      emulator.setDebug(5);
    else
      emulator.setDebug(debug);

    // FIXME: do I need the region?
    emulator.toFirmware(inputs.pfregion(), inputs.event().pv(), region, track, pv);

    emulator.run(inputs.pfregion(), out_ref);
    emulator.toFirmware(out_ref, egphs, egele);

    emulator.runIso(inputs.pfregion(), inputs.event().pvs, out_ref);
    emulator.toFirmware(out_ref, egphs_ref, egele_ref);

    #ifndef BOARD_none
          tkeg_tkisolation_pack_in(region, inputs.event().pv(), track, egphs, egele, packed_region, packed_pv, packed_track, packed_photons, packed_eles);
          packed_tkeg_tkisolation(packed_region, packed_pv, packed_track, packed_photons, packed_eles, packed_photons_out, packed_eles_out);
          tkeg_tkisolation_unpack_out(packed_photons_out, packed_eles_out, egphs, egele);
    #else
      if (doTkIso) {
        tkeg_tkisolation(region, pv, track, egphs, egele);
      }
    #endif

    int errors = 0;

    for (int it = 0; it < NEM_EGOUT; it++) {
      if (!egisoele_equals(egele_ref[it], egele[it], "EG Iso Ele", it))
        errors++;
      if (egele_ref[it].hwPt > 0)
        nEles_tests++;
    }

    for (int it = 0; it < NEM_EGOUT; it++) {
      if (!egiso_equals(egphs_ref[it], egphs[it], "EG Iso", it))
        errors++;
      if (egphs_ref[it].hwPt > 0)
        nPhotons_tests++;
    }

    if (errors != 0) {
      tot_errors += errors;
      printf("Error in computing test %d (%d)\n", test, errors);
      printf("    Passed %d tests (%d electrons, %d photons)\n", passed_tests, nEles_tests, nPhotons_tests);

      //     printf("Inputs: \n"); debugHR.dump_inputs(calo, track, mu);
      //     printf("Reference output: \n"); debugHR.dump_outputs(outch_ref, outne_ref, outmupf_ref);
      //     printf("Current output: \n"); debugHR.dump_outputs(outch, outne, outmupf);
      return 1;
    } else {
      passed_tests++;
      // printf("Passed test %d\n", test);
    }
  }
  std::cout << "# of errors: " << tot_errors << std::endl;
  printf("Passed %d tests (%d electrons, %d photons)\n", passed_tests, nEles_tests, nPhotons_tests);

  return 0;
}

#include <cstdio>
#include "firmware/pfeginput.h"

#include "ref/pfeginput_ref.h"

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
#endif

  // input TP objects
  HadCaloObj hadcalo[NCALO];

  // z0_t hwZPV;
  // MuObj mu[NMU];

  // output PF objects
  // EmCaloObj file_emcalo[NCALO];
  EmCaloObj sel_emcalo[NCALO];
  EmCaloObj sel_emcalo_ref[NCALO];

  ap_uint<EmCaloObj::BITWIDTH> sel_emcalo_packed[NCALO];

  l1ct::EGInputSelectorEmuConfig cfg(EGINPUT_BITMASK, NCALO, NCALO, true);
  l1ct::EGInputSelectorEmulator emulator(cfg);
  // -----------------------------------------
  // run multiple tests
  int passed_tests = 0;
  int n_tests = 0;
  int tot_errors = 0;

  for (int test = 1; test <= NTEST; ++test) {
    // std::cout << "------------ TEST: " << test << std::endl;
    // get the inputs from the input object
    if (!inputs.nextPFRegion())
      break;

    int debug = 0;
    if (test < 1)
      debug = 5;

    emulator.setDebug(debug);

    emulator.toFirmware(inputs.pfregion(), hadcalo);

    std::vector<EmCaloObjEmu> selemcalos;
    emulator.select_eginputs(inputs.pfregion().hadcalo, selemcalos);

    emulator.toFirmware(selemcalos, sel_emcalo_ref);

    // initialize output buffer
    for (int idx = 0; idx < NCALO; idx++) {
      sel_emcalo[idx].clear();
      sel_emcalo_packed[idx] = sel_emcalo[idx].pack();
      // std::cout << "[" << idx << "] sel_emcalo pt: " << sel_emcalo[idx].hwPt << " sel_emcalo_packed pt: " << EmCaloObj::unpack(sel_emcalo_packed[idx]).hwPt <<  std::endl;

    }

    int outidx = 0;
    for (int inid = 0; inid < NCALO; inid++) {
      const HadCaloObj had_in = hadcalo[inid];
      bool valid_in = true;
      if (had_in.hwPt == 0)
        valid_in = false;

      bool valid_out = false;

#ifndef BOARD_none
      packed_select_eginput(had_in.pack(), valid_in, sel_emcalo_packed[outidx], valid_out);
      sel_emcalo[outidx] = EmCaloObj::unpack(sel_emcalo_packed[outidx]);

#else
      select_eginput(had_in, valid_in, sel_emcalo[outidx], valid_out);
#endif
      // std::cout << "[" << inid << "] IN pt: " << had_in.hwPt << " [" << outidx << "]"<< " OUT pt: " << sel_emcalo[outidx].hwPt << " VALID: " << valid_out << std::endl;

      if (valid_out)
        outidx++;
    }

    int errors = 0;

    for (int it = 0; it < NCALO; it++) {
      if (!em_equals(sel_emcalo_ref[it], sel_emcalo[it], "selected EM calos", it))
        errors++;
      if (sel_emcalo_ref[it].hwPt > 0)
        n_tests++;
    }

    if (errors != 0) {
      tot_errors += errors;
      printf("Error in computing test %d (%d)\n", test, errors);
      printf("    Passed %d tests (%d selected clusters)\n", passed_tests, n_tests);

      return 1;
    } else {
      passed_tests++;
      // printf("Passed test %d\n", test);
    }
  }
  std::cout << "# of errors: " << tot_errors << std::endl;
  printf("Passed %d tests (%d selected clusters)\n", passed_tests, n_tests);

  return 0;
}

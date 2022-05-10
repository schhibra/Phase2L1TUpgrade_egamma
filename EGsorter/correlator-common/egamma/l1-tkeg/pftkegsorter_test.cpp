#include "firmware/pftkegsorter.h"
#include "ref/pftkegsorter_ref.h"

#include "../../utils/DumpFileReader.h"
#include "../../utils/test_utils.h"

int main(int argc, char *argv[]) {
  bool debug = true;

  // Read dumps
  DumpFileReader inputs(argv[1]);

  for (int iEvent=0, nEvent=10; iEvent<nEvent; ++iEvent) {

    // Initialize input/output objects
    int errors_pho = 0;
    int errors_ele = 0;
    
    // emulator
    std::vector<std::vector<l1ct::EGIsoObjEmu> > emu_photons_sorted_inBoard(5);
    std::vector<std::vector<l1ct::EGIsoEleObjEmu> > emu_electrons_sorted_inBoard(5);

    // synthesis
    bool newEvent = true;

    l1ct::EGIsoObj photons_sorted[NOBJSORTED];
    l1ct::EGIsoEleObj electrons_sorted[NOBJSORTED];

#ifndef BOARD_none
    ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_photon;
    ap_uint<l1ct::EGIsoEleObj::BITWIDTH> packed_electron;
    ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_photons_sorted[NOBJSORTED];
    ap_uint<l1ct::EGIsoEleObj::BITWIDTH> packed_electrons_sorted[NOBJSORTED];
#endif

    if (debug) {
      std::cout << std::endl;
      std::cout << std::endl;
      std::cout << "###########" << std::endl;
      std::cout << "  EVENT " << iEvent << std::endl;
      std::cout << "###########" << std::endl;
      std::cout << std::endl;
    }

    if (!inputs.nextEvent()) break;

    l1ct::PFTkEGSorterEmulator emulator_pho/*(splitEGObjectsToBoards=false*//*, nObjToSort=6*//*, nObjSorted=54*//*, nPFRegions=54*//*, nBoards=5)*/;
    emulator_pho.setDebug(debug);
    emulator_pho.run<l1ct::EGIsoObjEmu>(inputs.event().pfinputs, inputs.event().out, emu_photons_sorted_inBoard);

    l1ct::PFTkEGSorterEmulator emulator_ele/*(splitEGObjectsToBoards=false*//*, nObjToSort=6*//*, nObjSorted=54*//*, nPFRegions=54*//*, nBoards=5)*/;
    emulator_ele.setDebug(debug);
    emulator_ele.run<l1ct::EGIsoEleObjEmu>(inputs.event().pfinputs, inputs.event().out, emu_electrons_sorted_inBoard);

    for (int i=0, ni=inputs.event().out.size(); i<ni; i++) {

      l1ct::EGIsoObj photons[NOBJTOSORT];
      l1ct::EGIsoEleObj electrons[NOBJTOSORT];
      
      l1ct::toFirmware(inputs.event().out[i].egphoton, NOBJTOSORT, photons);
      l1ct::toFirmware(inputs.event().out[i].egelectron, NOBJTOSORT, electrons);

      for (int j=0; j<NOBJTOSORT; j++) {
#ifndef BOARD_none
        pftkegsorter_pack_in<l1ct::EGIsoObj>(photons[j], packed_photon); //pack_in
	packed_pftkegsorter_pho(newEvent, packed_photon, packed_photons_sorted);

        pftkegsorter_pack_in<l1ct::EGIsoEleObj>(electrons[j], packed_electron); //pack_in
	packed_pftkegsorter_ele(newEvent, packed_electron, packed_electrons_sorted);
#else
	pftkegsorter_pho(newEvent, photons[j], photons_sorted);
	pftkegsorter_ele(newEvent, electrons[j], electrons_sorted);
#endif

	if(newEvent) newEvent = false;
      }
    }
#ifndef BOARD_none
    pftkegsorter_unpack_out<l1ct::EGIsoObj,NOBJSORTED>(packed_photons_sorted, photons_sorted); //unpack_out
    pftkegsorter_unpack_out<l1ct::EGIsoEleObj,NOBJSORTED>(packed_electrons_sorted, electrons_sorted); //unpack_out
#endif

    for(int it=0; it<NOBJSORTED; it++) {
      if(!egiso_equals(emu_photons_sorted_inBoard[0][it], photons_sorted[it], "EG Iso", it)) errors_pho++;
    }
    std::cout << "Total errors for photons in Event "<<iEvent<<": "<<errors_pho<<"\n";

    for(int it=0; it<NOBJSORTED; it++) {
      if(!egisoele_equals(emu_electrons_sorted_inBoard[0][it], electrons_sorted[it], "EG Iso Ele", it)) errors_ele++;
    }
    std::cout << "Total errors for electrons in Event "<<iEvent<<": "<<errors_ele<<"\n";
  }

  return 0;
}

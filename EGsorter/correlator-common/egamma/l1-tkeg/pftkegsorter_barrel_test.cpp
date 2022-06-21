#include "firmware/pftkegsorter_barrel.h"
#include "ref/pftkegsorter_barrel_ref.h"

#include "../../utils/DumpFileReader.h"
#include "../../utils/test_utils.h"

using namespace l1ct;

void regionIndexer(unsigned int NofBoards, unsigned int regPerBoard, std::vector<std::vector<unsigned int>>& region_index) {
  region_index.resize(NofBoards);
  for (unsigned int i=0; i<NofBoards; i++) {
    for (unsigned int j=regPerBoard*i; j<regPerBoard*(i+1); j++){
      region_index[i].push_back(j);
    }
  }
}

int main(int argc, char *argv[]) {
  bool debug = false;

  // Split arguments
  std::string text = argv[1];
  std::string comma_delimiter = ",";
  std::vector<std::string> args{};

  size_t pos;
  do {
    pos = text.find(comma_delimiter);
    args.push_back(text.substr(0, pos));
    text.erase(0, pos + comma_delimiter.length());
  }
  while (pos != std::string::npos);

  // Read dumps
  std::string dumpFileStr = args[0]+"_"+args[1]+".dump";
  const char *dumpFileName = dumpFileStr.c_str();
  DumpFileReader inputs(dumpFileName);

  // Emulate CMSSW board configuration
  std::vector<std::vector<unsigned int>> region_index;
  if(args[1]=="Barrel") regionIndexer(3, 18, region_index);
  else if(args[1]=="HGCal") regionIndexer(2, 9, region_index);
  else if(args[1]=="HGCalNoTK") regionIndexer(1, 18, region_index);
  else if(args[1]=="HF") regionIndexer(1, 18, region_index); // Dummy, the configuration is empty in CMSSW
  else {
    std::cout << "Unknown region: Please check!" << "\n";
    return 1000;
  }

  unsigned int nElectrons_tests = 0;
  unsigned int nPhotons_tests = 0;
  unsigned int tot_errors_pho = 0;
  unsigned int tot_errors_ele = 0;
  unsigned int tot_errors_validout = 0;

  bool validout = false;

  for (unsigned int iEvent=0, nEvent=1; iEvent<nEvent; ++iEvent) {
    
    if (!inputs.nextEvent()) break;
    
    if (debug) {
      std::cout << std::endl;
      std::cout << std::endl;
      std::cout << "###########" << std::endl;
      std::cout << "  EVENT " << iEvent << std::endl;
      std::cout << "###########" << std::endl;
      std::cout << std::endl;
    }

    for (unsigned int iboard=0; iboard<region_index.size(); iboard++) {
    //for (unsigned int iboard=0; iboard<1; iboard++) {

      if (debug) for (const int i: region_index[iboard]) std::cout<<"region index "<<i<<"\n";

      // Initialize input/output objects
      unsigned int errors_pho = 0;
      unsigned int errors_ele = 0;
      unsigned int errors_validout = 0;

      // emulator /////////////////////////////////////////////
      std::vector<l1ct::EGIsoObjEmu> emu_photons_sorted_inBoard;
      std::vector<l1ct::EGIsoEleObjEmu> emu_electrons_sorted_inBoard;

      l1ct::PFTkEGSorterBarrelEmulator emulator_pho(NOBJTOSORT, NOBJSORTED);
      emulator_pho.setDebug(debug);
      emulator_pho.run(inputs.event().pfinputs, inputs.event().out, region_index[iboard], emu_photons_sorted_inBoard);
	
      l1ct::PFTkEGSorterBarrelEmulator emulator_ele(NOBJTOSORT, NOBJSORTED);
      emulator_ele.setDebug(debug);
      emulator_ele.run(inputs.event().pfinputs, inputs.event().out, region_index[iboard], emu_electrons_sorted_inBoard);//<===
      /////////////////////////////////////////////////////////
      
      // firmware /////////////////////////////////////////////
      bool newBoard = true;

      ap_uint<l1ct::EGIsoObj::BITWIDTH> photons_sorted[NOBJSORTED];
      ap_uint<l1ct::EGIsoEleObj::BITWIDTH> electrons_sorted[NOBJSORTED];

      for (unsigned int i: region_index[iboard]) {
	
	if (debug) std::cout<<"region-index "<<i<<"\n";//0 to 53 for barrel
	  
	bool lastregion = false; if (region_index[iboard].size() %2 == 1 && i == region_index[iboard].size()-1) lastregion = true;

	l1ct::PFRegion region = inputs.event().pfinputs[i].region;
      	l1ct::EGIsoObj photons_in[NOBJTOSORT];
        l1ct::EGIsoEleObj electrons_in[NOBJTOSORT];
	
      	emulator_pho.toFirmware_pho(inputs.event().out[i], photons_in);
      	emulator_ele.toFirmware_ele(inputs.event().out[i], electrons_in);//<===

	ap_uint<l1ct::PFRegion::BITWIDTH> packed_region;
      	ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_photons_in[NOBJTOSORT];
      	ap_uint<l1ct::EGIsoEleObj::BITWIDTH> packed_electrons_in[NOBJTOSORT];
      	pftkegsorter_barrel_pack_in(region, photons_in, packed_region, packed_photons_in);
      	pftkegsorter_barrel_pack_in(region, electrons_in, packed_region, packed_electrons_in);//<===

	packed_pftkegsorter_barrel_pho(newBoard, lastregion, packed_region, packed_photons_in, photons_sorted, region_index[iboard].size(), validout);
      	packed_pftkegsorter_barrel_ele(newBoard, lastregion, packed_region, packed_electrons_in, electrons_sorted, region_index[iboard].size(), validout);//<===

      	if(newBoard) newBoard = false;

	if (debug) std::cout<<" validout "<<validout<<"\n";
	if (i != (iboard+1)*region_index[iboard].size()-1 and validout == true) errors_validout++;
      }

      l1ct::EGIsoObj photons_out[NOBJSORTED];
      l1ct::EGIsoEleObj electrons_out[NOBJSORTED];
      pftkegsorter_barrel_unpack_out(photons_sorted, photons_out);
      pftkegsorter_barrel_unpack_out(electrons_sorted, electrons_out);//<===
      /////////////////////////////////////////////////////////

      for(int it = 0; it < NOBJSORTED; it++) {
      	if(!egiso_equals(emu_photons_sorted_inBoard[it], photons_out[it], "EG Iso", it)) {
	  errors_pho++;
	  if (debug) std::cout<<"event where sorted photons emu vs synth don't match "<<iEvent<<"\n";
	}
      	if(emu_photons_sorted_inBoard[it].hwPt > 0) nPhotons_tests++;
      }
      for(int it = 0; it < NOBJSORTED; it++) {
      	if(!egiso_equals(emu_electrons_sorted_inBoard[it], electrons_out[it], "EG Iso", it)) {
      	  errors_ele++;
      	  if (debug) std::cout<<"event where sorted electrons emu vs synth don't match "<<iEvent<<"\n";
      	}
      	if(emu_electrons_sorted_inBoard[it].hwPt > 0) nElectrons_tests++;
      }
      tot_errors_pho += errors_pho;
      tot_errors_ele += errors_ele;
      tot_errors_validout += errors_validout;
    }
  }
  std::cout<<"nElectrons_tests "<<nElectrons_tests<<" nPhotons_tests "<<nPhotons_tests<<"\n";
  std::cout<<"tot_errors_ele "<<tot_errors_ele<<" tot_errors_pho "<<tot_errors_pho<<"\n";
  std::cout<<"tot_errors_validout "<<tot_errors_validout<<"\n";

  if (tot_errors_ele > 0 || tot_errors_pho > 0 || tot_errors_validout > 0) return 1000;
  return 0;
}

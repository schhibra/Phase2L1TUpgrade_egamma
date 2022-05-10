#include "firmware/pftkegsorter_barrel.h"
#include "ref/pftkegsorter_barrel_ref.h"

#include "../../utils/DumpFileReader.h"
#include "../../utils/test_utils.h"

using namespace l1ct;

//const unsigned int NOBJ = 16;

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

  int passed_tests = 0;
  int nEles_tests = 0;
  int nPhotons_tests = 0;
  int tot_errors = 0;

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
    return 0;
  }

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

    //for (unsigned int iboard=0; iboard<region_index.size(); iboard++) {
    for (unsigned int iboard=0; iboard<1; iboard++) {

      if (debug) for (const int i: region_index[iboard]) std::cout<<"region index "<<i<<"\n";

      // Initialize input/output objects
      unsigned int errors_pho = 0;
      unsigned int errors_ele = 0;
      
      // emulator
      std::vector<l1ct::EGIsoObjEmu> emu_photons_sorted_inBoard;
      l1ct::PFTkEGSorterBarrelEmulator emulator_pho(NOBJ, NOBJ);
      
      emulator_pho.setDebug(debug);
      emulator_pho.run<l1ct::EGIsoObjEmu>(inputs.event().pfinputs, inputs.event().out, region_index[iboard], emu_photons_sorted_inBoard);
	
      // firmware
      bool newEvent = true;
            
      ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_photons_out[NOBJ];
      ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_eles_out[NOBJ];//<=========== EGIsoEleObj
      
      for (unsigned int id = region_index[iboard][0], idn = region_index[iboard][0]+1; id < (iboard+1)*region_index[iboard].size(); id += 2, idn = id + 1) {
	
	 if (debug) std::cout<<"region-index id "<<id<<" idn "<<idn<<"\n";

	bool oddregion = false;
      	std::vector<l1ct::PFInputRegion> per_pfreg_in;
      	std::vector<l1ct::OutputRegion> per_outreg_in;
	if (region_index[iboard].size() %2 == 1 && idn >= (iboard+1)*region_index[iboard].size()) {
	  if (debug) std::cout<<"odd region index "<<id<<"\n";
	  per_pfreg_in.push_back(inputs.event().pfinputs[id]);
	  per_pfreg_in.push_back(inputs.event().pfinputs[id]);
	  per_outreg_in.push_back(inputs.event().out[id]);
	  per_outreg_in.push_back(inputs.event().out[id]);
	  oddregion = true;
	}
	else {	  
	 if (debug) std::cout<<"region-index id "<<id<<" idn "<<idn<<"\n";
	  per_pfreg_in.push_back(inputs.event().pfinputs[id]);
	  per_pfreg_in.push_back(inputs.event().pfinputs[idn]);
	  per_outreg_in.push_back(inputs.event().out[id]);
	  per_outreg_in.push_back(inputs.event().out[idn]);
	}
	
      	l1ct::EGIsoObj photons_in[2][NOBJ];
        l1ct::EGIsoObj eles_in[2][NOBJ];
      	emulator_pho.toFirmware(per_pfreg_in, per_outreg_in, photons_in, eles_in);

      	ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_photons_in[2][NOBJ];
      	ap_uint<l1ct::EGIsoObj::BITWIDTH> packed_eles_in[2][NOBJ];//<=========== EGIsoEleObj
      	l2egsorter_pack_in(photons_in, eles_in, packed_photons_in, packed_eles_in);

      	packed_l2egsorter(newEvent, oddregion, packed_photons_in, packed_eles_in, packed_photons_out, packed_eles_out);

      	if(newEvent) newEvent = false;
      }

      l1ct::EGIsoObj photons_out[NOBJ];
      l1ct::EGIsoObj eles_out[NOBJ];//<=========== EGIsoEleObj
      l2egsorter_unpack_out(packed_photons_out, packed_eles_out, photons_out, eles_out);
      
      for(int it = 0; it < NL2_EGOUT; it++) {
      	if(!egiso_equals(emu_photons_sorted_inBoard[it], photons_out[it], "EG Iso", it)) errors_pho++;
      	if(emu_photons_sorted_inBoard[it].hwPt > 0) nPhotons_tests++;
      }
    }
  }
  return 0;
}

#ifndef REF_PFTKEGSORTER_BARREL_REF_H
#define REF_PFTKEGSORTER_BARREL_REF_H

#include <cstdio>
#include <vector>

//#ifdef CMSSW_GIT_HASH
//#include "FWCore/ParameterSet/interface/ParameterSet.h"
//#include "../dataformats/layer1_multiplicities.h"
//#include "../dataformats/layer1_emulator.h"
//#else
#include "../../../dataformats/layer1_multiplicities.h"
#include "../../../dataformats/layer1_emulator.h"
//#endif

#include "../../../common/bitonic_hybrid_sort_ref.h"

namespace edm {
  class ParameterSet;
}

//const unsigned int NREG = 9;
const unsigned int NOBJ = 16;

namespace l1ct {
  class PFTkEGSorterBarrelEmulator {
  public:
  PFTkEGSorterBarrelEmulator(const unsigned int nObjToSort = NOBJ, const unsigned int nObjSorted = NOBJ)
    : nObjToSort_(nObjToSort), nObjSorted_(nObjSorted), debug_(false) {}
    
    PFTkEGSorterBarrelEmulator(const edm::ParameterSet &iConfig);
    
    virtual ~PFTkEGSorterBarrelEmulator() {}

    void setDebug(bool debug = true) { debug_ = debug; };

    void toFirmware(const std::vector<PFInputRegion>& pfregions,
		    const std::vector<OutputRegion> &in,
                    EGIsoObj (&photons_in)[2][NOBJ],
                    EGIsoObj (&eles_in)[2][NOBJ]) const {//<=========== EGIsoEleObj
      for (unsigned int ib = 0; ib < 2; ib++) {
        for (unsigned int io = 0; io < NOBJ; io++) {
          EGIsoObj pho;
          EGIsoObj ele;//<=========== EGIsoEleObj
          if (io < in[ib].egphoton.size()) {
            pho = in[ib].egphoton[io];
	    //pho.hwEta = pfregions[io].region.hwGlbEta(pho.hwEta);;
	  }
          else
            pho.clear();
	  if (io < in[ib].egphoton.size()) {//<=========== egelectron
	    ele = in[ib].egphoton[io];//<=========== egelectron
	    //ele.hwEta = pfregions[io].region.hwGlbEta(ele.hwEta);;
	  }
          else
            ele.clear();
	  
          photons_in[ib][io] = pho;
          eles_in[ib][io] = ele;
        }
      }
    }

    template <typename T>//l1ct::EGIsoObjEmu
      void run(const std::vector<PFInputRegion>& pfregions,
    	       const std::vector<OutputRegion>& outregions,
    	       const std::vector<unsigned int>& region_index,
    	       std::vector<T>& eg_sorted_inBoard) {

      // we copy to be able to resize them
      std::vector<std::vector<EGIsoObjEmu>> photons_in;
      photons_in.reserve(nObjToSort_);
      for (unsigned int i : region_index) {
    	std::vector<T> photons;
    	extractEGObjEmu(pfregions[i].region, outregions[i], photons);
    	if (debug_) std::cout<<"photons size "<<photons.size()<<"\n";
    	resize_input(photons);
    	photons_in.push_back(photons);
    	if (debug_) std::cout<<"photons (re)size and total photons size "<<photons.size()<<" "<<photons_in.size()<<"\n";
      }
      
      merge(photons_in, eg_sorted_inBoard);
	
      if (debug_) {
	std::cout<<"photons.size() size "<<eg_sorted_inBoard.size()<<"\n";
    	for (const auto &out : eg_sorted_inBoard) std::cout<<"kinematics of sorted objetcs "<<out.hwPt<<" "<<out.hwEta<<" "<<out.hwPhi<<"\n";
      }
    }

  private:

    void extractEGObjEmu(const PFRegionEmu& region,
                         const l1ct::OutputRegion& outregion,
                         std::vector<l1ct::EGIsoObjEmu>& eg) {
      extractEGObjEmu(region, outregion.egphoton, eg);
    }
    void extractEGObjEmu(const PFRegionEmu& region,
                         const l1ct::OutputRegion& outregion,
                         std::vector<l1ct::EGIsoEleObjEmu>& eg) {
      extractEGObjEmu(region, outregion.egelectron, eg);
    }
    template <typename T>
      void extractEGObjEmu(const PFRegionEmu& region,
			   const std::vector<T>& regional_objects,
			   std::vector<T>& global_objects) {
      for (const auto& reg_obj : regional_objects) {
        global_objects.emplace_back(reg_obj);
        //global_objects.back().hwEta = region.hwGlbEta(reg_obj.hwEta);//<=========== uncomment this
        //global_objects.back().hwPhi = region.hwGlbPhi(reg_obj.hwPhi);//<=========== uncomment this
      }
    }
	
    template <typename T>
      void resize_input(std::vector<T> &in) const {
      if (in.size() > nObjToSort_) {
	in.resize(nObjToSort_);
      } else if (in.size() < nObjToSort_) {
	for (unsigned int i = 0, diff = (nObjToSort_ - in.size()); i < diff; ++i) {
	  in.push_back(T());
	  in.back().clear();
	}
      }
    }

    template <typename T>
      void merge_regions(const std::vector<T> &in_region1,
			 const std::vector<T> &in_region2,
			 std::vector<T> &out,
			 unsigned int nOut) const {
      // we crate a bitonic list
      out = in_region1;
      if (debug_) for (const auto &tmp : out) std::cout<<"out "<<tmp.hwPt<<"\n";
      std::reverse(out.begin(), out.end());
      if (debug_) for (const auto &tmp : out) std::cout<<"out reverse "<<tmp.hwPt<<"\n";      
      std::copy(in_region2.begin(), in_region2.end(), std::back_inserter(out));
      if (debug_) for (const auto &tmp : out) std::cout<<"out inserted "<<tmp.hwPt<<"\n";

      hybridBitonicMergeRef(&out[0], out.size(), 0, false);
      if (debug_) for (const auto &tmp : out) std::cout<<"final "<<tmp.hwPt<<"\n";

      if (out.size() > nOut)
	out.resize(nOut);
    }

    template <typename T>
      void merge(const std::vector<std::vector<T>> &in_objs, std::vector<T> &out) const {
      if (in_objs.size() == 1) {//size is 1, fine!
	std::copy(in_objs[0].begin(), in_objs[0].end(), std::back_inserter(out));
	if (out.size() > nObjSorted_)
	  out.resize(nObjSorted_);
      }
      else if (in_objs.size() == 2) {//size is 2, fine!
	merge_regions(in_objs[0], in_objs[1], out, nObjSorted_);
      }
      else {		
	std::vector<T> pair_merge_01;//size is >2, merge 0 and 1 regions always
	merge_regions(in_objs[0], in_objs[1], pair_merge_01, nObjToSort_);

	std::vector<std::vector<T>> to_merge;
	if (in_objs.size() == 3) to_merge.push_back(pair_merge_01);//push 01 only if size is 3 //and then in_objs[id] will be pushed into it

	std::vector<T> pair_merge_tmp = pair_merge_01;//
	for (unsigned int id = 2, idn = 3; id < in_objs.size(); id += 2, idn = id + 1) {
          if (idn >= in_objs.size()) {//if size is odd number starting from 3
            to_merge.push_back(in_objs[id]);
	  }
	  else {
	    std::vector<T> pair_merge;
	    merge_regions(in_objs[id], in_objs[idn], pair_merge, nObjToSort_);//merge two regions: 23, 45, 67, and so on
 
	    merge_regions(pair_merge_tmp, pair_merge, pair_merge_tmp, nObjToSort_);//merge 23 with 01 for first time // then merge 45, and then 67, and then 89, and so on  
	    to_merge.push_back(pair_merge_tmp);//push back 0123, 012345, 01234567, and so on (remember 01 is the 0th element)
	  }
	}
	if (in_objs.size() % 2 == 1) merge_regions(to_merge[to_merge.size()-2], to_merge[to_merge.size()-1], out, nObjToSort_);//e.g. is size is 3, we merge 01 and 2, if size is 7, we merge 012345 and 6
	else out = pair_merge_tmp;//if size is even number, e.g. 6, out is 012345 (to_merge[to_merge.size()-1])  
      }
    }

    const unsigned int nObjToSort_;
    const unsigned int nObjSorted_;
    bool debug_;
  };
}

#endif

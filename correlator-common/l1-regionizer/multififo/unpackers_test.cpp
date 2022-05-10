#include "../../utils/test_utils.h"
#include "../../utils/DumpFileReader.h"
#include "firmware/dummy_obj_unpackers.h"
#include "utils/dummy_obj_packers.h"

#include <cstdlib>
#include <cstdio>
#include <vector>

#ifndef NTEST
#define NTEST 48
#endif

template<unsigned int NB>
struct word { 
    ap_uint<NB> data; 
    bool valid;
};

void distribute(const std::vector<ap_uint<64>> & in, std::vector<word<64>> channels[3]) {
    unsigned int nrows = (in.size()+2)/3 + 2; // add a few nulls at the end
    for (unsigned int c = 0; c < 3; ++c) {
        channels[c].resize(nrows);
        for (auto & w : channels[c]) { w.data = 0; w.valid = false; }
    }
    for (unsigned int i = 0, n = in.size(); i < n; ++i) {
        channels[i%3][i/3].data = in[i];
        channels[i%3][i/3].valid = true;
    }
}

template<unsigned int NB, typename Obj, typename Sec>
bool check_equal(const word<NB> & p, const Obj & o, int itest, const Sec & sec, const char *what, int iframe, int i) {
    if (!p.valid) {
        printf("ERROR at event %4d, sector ieta %+4d iphi %+4d, %s at frame %3d, index %d: found null when %s expected\n",
                itest, sec.region.intEtaCenter(), sec.region.intPhiCenter(), what, iframe, i, o.pack().to_string(16).c_str());
        return false;
    }
    ap_uint<NB> op = o.pack();
    if (p.data != op) {
        printf("ERROR at event %4d, sector ieta %+4d iphi %+4d, %s at frame %3d, index %d: found %s when %s expected\n",
                itest, sec.region.intEtaCenter(), sec.region.intPhiCenter(), what, iframe, i, 
                p.data.to_string(16).c_str(), op.to_string(16).c_str());
        return false;
    }
    return true;
}

template<unsigned int NB, typename Sec>
bool check_null(const word<NB> & p, int itest, const Sec & sec, const char *what, int iframe, int i) {
    if (p.valid) {
        printf("ERROR at event %4d, sector ieta %+4d iphi %+4d, %s at frame %3d, index %d: found %s when null expected\n",
                itest, sec.region.intEtaCenter(), sec.region.intPhiCenter(), what, iframe, i, p.data.to_string(16).c_str());
        return false;
    }
    return true;
}


int main(int argc, char **argv) {
    DumpFileReader inputs("TTbar_PU200_HGCal.dump");

    std::vector<word<64>> channels[3];
    bool ok = true;

    for (int itest = 0; itest < NTEST; ++itest) {
        if (!inputs.nextEvent()) break;
        printf("Processing event %d\n", itest);

        for (auto & sec : inputs.event().decoded.track) {
            distribute(pack_tracks(sec), channels);
            word<72> tk_out[2];
            for (unsigned int iframe = 0, nframes = channels[0].size(); iframe < nframes; ++iframe) {
                unpack_track_3to2(channels[0][iframe].data, channels[0][iframe].valid,
                                  channels[1][iframe].data, channels[1][iframe].valid,
                                  channels[2][iframe].data, channels[2][iframe].valid,
                                  tk_out[0].data, tk_out[0].valid,
                                  tk_out[1].data, tk_out[1].valid);
                for (unsigned int i = 0; i < 2; ++i) {
                    unsigned int itk = iframe*2+i;
                    if (itk < sec.size()) {
                        if (!check_equal(tk_out[i], sec[itk], itest, sec, "track", iframe, i)) ok = false;
                    } else {
                        if (!check_null(tk_out[i], itest, sec, "track", iframe, i)) ok = false;
                    }
                }
            } 

            if (!ok) break;
        }

        
        for (auto & sec : inputs.event().decoded.hadcalo) {
            distribute(pack_hgcal(sec), channels);
            word<72> hgc_out;
            for (unsigned int iframe = 0, nframes = channels[0].size(); iframe < nframes; ++iframe) {
                unpack_hgcal_3to1(channels[0][iframe].data, channels[0][iframe].valid,
                                  channels[1][iframe].data, channels[1][iframe].valid,
                                  channels[2][iframe].data, channels[2][iframe].valid,
                                  hgc_out.data, hgc_out.valid);
                unsigned int i = iframe;
                if (iframe < sec.size()) {
                    if (!check_equal(hgc_out, sec[iframe], itest, sec, "hgcal", iframe, 0)) ok = false;
                } else {
                    if (!check_null(hgc_out, itest, sec, "hgcal", iframe, 0)) ok = false;
                }
            } 

            if (!ok) break;
        }

        auto & sec = inputs.event().decoded.muon;
        distribute(pack_muons(sec), channels);
        word<72> mu_out[2];
        for (unsigned int iframe = 0, nframes = channels[0].size(); iframe < nframes; ++iframe) {
            unpack_mu_3to12(channels[0][iframe].data, channels[0][iframe].valid,
                            channels[1][iframe].data, channels[1][iframe].valid,
                            channels[2][iframe].data, channels[2][iframe].valid,
                            mu_out[0].data, mu_out[0].valid,
                            mu_out[1].data, mu_out[1].valid);
            for (unsigned int i = 0; i < 2; ++i) {
                unsigned int imu = (iframe*3)/2+i;
                if (imu < sec.size() && i <= (iframe % 2)) {
                    if (!check_equal(mu_out[i], sec[imu], itest, sec, "mu", iframe, i)) ok = false;
                } else {
                    if (!check_null(mu_out[i], itest, sec, "mu", iframe, i)) ok = false;
                }
            }
        }

        if (!ok) break;
    }

    if (!ok) return 1;

    printf("All tests completed\n");
    return 0;

}

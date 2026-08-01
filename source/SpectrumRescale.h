//
// Created by arden on 7/31/26.
//

#ifndef MAIM_SPECTRUMRESCALE_H
#define MAIM_SPECTRUMRESCALE_H

#include <cmath>

// Shared by Mp3ControllerManager (lame/blade) and OpusController: compresses a spectrum
// magnitude that's roughly 0 to 1(+) onto a 0-1 log scale for the MDCTGraph display.
inline float rescaleMDCT(const float a)
{
    if (a < (std::pow(10.f, -10.f))) {
        return 0;
    } else if (a > 1) {
        return 1;
    } else {
        return std::log10(a) / 10 + 1;
    }
}

#endif //MAIM_SPECTRUMRESCALE_H

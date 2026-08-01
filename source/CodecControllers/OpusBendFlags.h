//
// Created by arden on 7/31/26.
//

#ifndef MAIM_OPUSBENDFLAGS_H
#define MAIM_OPUSBENDFLAGS_H

#include <array>

// Mirrors the role of BendFlagsAndData (lame) / blade_bend_flags (blade): a single struct
// of "bend" parameters that gets threaded through the encoder to warp the encoding process
// away from the codec's intended behaviour. Opus has no exposed MDCT stage of its own (libopus
// is used unmodified), so these bends are applied to our own FFT of the PCM signal that Opus
// consumes/produces, standing in for the frequency-domain hooks lame/blade have natively.
struct OpusBendFlags
{
    // Frequency reassignment: mdctBandReassignment[destBand] = sourceBand, same 32-band
    // remapping idea as BendFlagsAndData::mdct_band_reassignments.
    static constexpr int numReassignmentBands = 32;
    std::array<int, numReassignmentBands> mdctBandReassignment = identityReassignment();

    // Spectral effects: horizontal (frequency) and vertical (amplitude) shift, matching
    // BendFlagsAndData::mdct_post_h_shift / mdct_post_v_shift.
    int mdctPostHShift = 0;
    float mdctPostVShift = 0.0f;

    // Spectral effects: blends each frame's spectrum with the previous frame's, matching
    // BendFlagsAndData::mdct_feedback.
    float mdctFeedback = 0.0f;

    static std::array<int, numReassignmentBands> identityReassignment()
    {
        std::array<int, numReassignmentBands> order{};
        for (int i = 0; i < numReassignmentBands; ++i) {
            order[static_cast<size_t>(i)] = i;
        }
        return order;
    }

    bool isIdentity() const
    {
        return mdctBandReassignment == identityReassignment()
            && mdctPostHShift == 0
            && mdctPostVShift >= 0.0f && mdctPostVShift <= 0.0f
            && mdctFeedback >= 0.0f && mdctFeedback <= 0.0f;
    }
};

#endif //MAIM_OPUSBENDFLAGS_H

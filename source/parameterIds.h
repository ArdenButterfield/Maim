//
// Created by arden on 1/29/25.
//

#ifndef MAIM_PARAMETERIDS_H
#define MAIM_PARAMETERIDS_H

#include "juce_core/juce_core.h"

#define NUM_REASSIGNMENT_BANDS 20

const juce::String DRIVE_PARAM_ID = "drive";
const juce::String MAKEUP_GAIN_PARAM_ID = "makeupgain";
const juce::String BUTTERFLY_STANDARD_PARAM_ID = "butterflystandard";
const juce::String BUTTERFLY_CROSSED_PARAM_ID = "butterflycrossed";
const juce::String MDCT_STEP_PARAM_ID = "mdctstep";
const juce::String MDCT_INVERT_PARAM_ID = "mdctinvert";
const juce::String MDCT_PITCH_SHIFT_PARAM_ID = "mdctposthshift";
const juce::String MDCT_AMPLITUDE_SHIFT_PARAM_ID = "mdctpostvshift";
const juce::String MDCT_WINDOW_INCREMENT_PARAM_ID = "mdctwindowincr";
const juce::String MDCT_FEEDBACK_PARAM_ID = "mdctfeedback";
const juce::String TURBO_PARAM_ID = "turbo";
const juce::String THRESHOLD_BIAS_PARAM_ID = "thresholdbias";
const std::array<juce::String, NUM_REASSIGNMENT_BANDS> BAND_ORDER_PARAM_IDS = {
    "bandorder0",
    "bandorder1",
    "bandorder2",
    "bandorder3",
    "bandorder4",
    "bandorder5",
    "bandorder6",
    "bandorder7",
    "bandorder8",
    "bandorder9",
    "bandorder10",
    "bandorder11",
    "bandorder12",
    "bandorder13",
    "bandorder14",
    "bandorder15",
    "bandorder16",
    "bandorder17",
    "bandorder18",
    "bandorder19",
};

const juce::String PACKET_LOSS_RATE_PARAM_ID = "plossrate";
const juce::String PACKET_LOSS_PULSE_WIDTH_PARAM_ID = "plosswidth";
const juce::String PACKET_LOSS_JITTER_PARAM_ID = "plossjitter";
const juce::String PACKET_LOSS_BEAT_SYNC_PARAM_ID = "beatsync";
const juce::String PACKET_LOSS_STICK_PARAM_ID = "lossstick";

const juce::String BITRATE_PARAM_ID = "bitrate";
const juce::String ENCODER_PARAM_ID = "encoder";
const juce::String HI_CUT_PARAM_ID = "hicut";
const juce::String LO_CUT_PARAM_ID = "locut";
const juce::String MIX_PARAM_ID = "mix";

#endif //MAIM_PARAMETERIDS_H

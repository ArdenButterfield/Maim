//
// Created by arden on 7/23/23.
//

#include "DemureSlider.h"

void DemureSlider::mouseEnter(const juce::MouseEvent& e) {
    setAlpha(1.0f);
}

void DemureSlider::mouseExit(const juce::MouseEvent& e) {
    setAlpha(0.3f);
}
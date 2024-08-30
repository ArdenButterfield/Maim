//
// Created by arden on 7/23/23.
//

#include "DemureSlider.h"

void DemureSlider::mouseEnter(const juce::MouseEvent& e) {
    setAlpha(1.0f);
    if (label != nullptr) {
        label->setAlpha(1.0f);
        label->repaint();

    }
}

void DemureSlider::mouseExit(const juce::MouseEvent& e) {
    setAlpha(0.3f);
    if (label != nullptr) {
        label->setAlpha(0.3f);
        label->repaint();

    }
}
void DemureSlider::setLabel (juce::Label* _label)
{
    label = _label;
}

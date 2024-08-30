//
// Created by arden on 7/23/23.
//

#ifndef MAIM_DEMURESLIDER_H
#define MAIM_DEMURESLIDER_H

#include <juce_graphics/juce_graphics.h>
#include <juce_audio_processors/juce_audio_processors.h>


class DemureSlider : public juce::Slider
{
public:
    DemureSlider (SliderStyle style, TextEntryBoxPosition textBoxPosition) : juce::Slider(style, textBoxPosition), label(nullptr) {};
    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;
    void setLabel (juce::Label* label);
private:
    juce::Label* label;
};

#endif //MAIM_DEMURESLIDER_H

//
// Created by arden on 7/18/23.
//

#ifndef MAIM_MAIMLOOKANDFEEL_H
#define MAIM_MAIMLOOKANDFEEL_H

#pragma once

#include "BinaryData.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include "MaimColours.h"

// BEDSTEAD FONT
// https://www.1001fonts.com/bedstead-font.html
// Bedtead by Ben Harris.
// Released into the public domain.
// Thanks to: https://forum.juce.com/t/modern-custom-font-guide/20841/3

class MaimLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MaimLookAndFeel();
    ~MaimLookAndFeel() override;


    void drawRotarySlider (juce::Graphics& g,
        int x,
        int y,
        int width,
        int height,
        float sliderPos,
        const float rotaryStartAngle,
        const float rotaryEndAngle,
        juce::Slider&) override;

    void drawLinearSlider(juce::Graphics& g,
        int x,
        int y,
        int width,
        int height,
        float sliderPos,
        float minSliderPos,
        float maxSliderPos,
        const juce::Slider::SliderStyle style,
        juce::Slider& s
        ) override;

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
#if 0
    void drawComboBox (juce::Graphics& g,
                       int width,
                       int height,
                       const bool isButtonDown,
                       int buttonX,
                       int buttonY,
                       int buttonW,
                       int buttonH,
                       juce::ComboBox& box) override;
#endif
    const juce::Typeface::Ptr main_font;
    const juce::Typeface::Ptr bold_font;
    const juce::Typeface::Ptr tooltip_font;

private:
    const float pi = 3.14159265359;
};
#endif //MAIM_MAIMLOOKANDFEEL_H

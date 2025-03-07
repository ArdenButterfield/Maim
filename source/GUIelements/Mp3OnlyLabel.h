//
// Created by arden on 3/6/25.
//

#ifndef MAIM_MP3ONLYLABEL_H
#define MAIM_MP3ONLYLABEL_H

#include "juce_gui_basics/juce_gui_basics.h"
#include "MaimLookAndFeel.h"

class Mp3OnlyLabel : public juce::Label
{
public:
    Mp3OnlyLabel() : juce::Label() {
        setColour(juce::Label::textColourId, MaimColours::BEVEL_DARK);
        setFont(juce::Font(MaimLookAndFeel().main_font).withHeight(15.f));
        setText("Section for Lame/\nBlade only", juce::dontSendNotification);
        setJustificationType(juce::Justification::centred);
    }
};

#endif //MAIM_MP3ONLYLABEL_H

//
// Created by arden on 7/18/23.
//

#ifndef MAIM_TITLEPANEL_H
#define MAIM_TITLEPANEL_H

#include "StageWindow.h"

class TitlePanel  : public StageWindow
{
public:
    TitlePanel (juce::AudioProcessorValueTreeState& p);
    ~TitlePanel();
    void paint (juce::Graphics& g) override;
    void resized() override;
private:

    juce::HyperlinkButton manualLink;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TitlePanel)
};

#endif //MAIM_TITLEPANEL_H

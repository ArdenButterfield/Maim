/*
  ==============================================================================

    PostSection.cpp
    Created: 7 Apr 2023 9:21:27am
    Author:  Arden Butterfield

  ==============================================================================
*/

#include <JuceHeader.h>
#include "PostSection.h"

//==============================================================================

void PostSection::resized()
{
    postFilterSlider.setBounds(getLocalBounds());
}

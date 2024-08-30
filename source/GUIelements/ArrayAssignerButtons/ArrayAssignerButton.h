//
// Created by arden on 7/23/23.
//

#ifndef MAIM_ARRAYASSIGNERBUTTON_H
#define MAIM_ARRAYASSIGNERBUTTON_H

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_graphics/juce_graphics.h>

enum ButtonDesigns {
    RESET,
    RANDOM,
    SHIFT_UP,
    SHIFT_DOWN
};

class ArrayAssignerButton : public juce::Button
{
public:
    ArrayAssignerButton(const juce::String& name, ButtonDesigns d) : juce::Button(name), buttonDesigns(d), randomShapeIndex(4) {}
    void paintButton(juce::Graphics &g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

private:
    static const int buttonPixelSize = 3;
    static const int buttonPixelDims = 9;
    static const int shapeSize = buttonPixelSize * buttonPixelDims;

    void drawPattern(juce::Graphics &g, juce::Rectangle<int> bounds, const std::array<char, buttonPixelDims * buttonPixelDims>& arr);
    void drawResetShape(juce::Graphics &g, juce::Rectangle<int> bounds);
    void drawRandomShape(juce::Graphics &g, juce::Rectangle<int> bounds);
    void drawShiftUpShape(juce::Graphics &g, juce::Rectangle<int> bounds);
    void drawShiftDownShape(juce::Graphics &g, juce::Rectangle<int> bounds);

    void mouseUp (const juce::MouseEvent &) override;

    const ButtonDesigns buttonDesigns;

    const std::array<char, buttonPixelDims * buttonPixelDims> resetShape {
        0, 0, 0, 0, 0, 0, 0, 0, 1,
        0, 0, 0, 0, 0, 0, 0, 1, 0,
        0, 0, 0, 0, 0, 0, 1, 0, 0,
        0, 0, 0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 0, 1, 0, 0, 0, 0,
        0, 0, 0, 1, 0, 0, 0, 0, 0,
        0, 0, 1, 0, 0, 0, 0, 0, 0,
        0, 1, 0, 0, 0, 0, 0, 0, 0,
        1, 0, 0, 0, 0, 0, 0, 0, 0
    };

    juce::Random random;
    int randomShapeIndex;

    const std::array<std::array<char, buttonPixelDims * buttonPixelDims>, 6> randomShapes {
        std::array<char, buttonPixelDims * buttonPixelDims>{
            0, 1, 1, 1, 1, 1, 1, 1, 0,
            1, 0, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 0, 0, 1, 0, 0, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 0, 1,
            0, 1, 1, 1, 1, 1, 1, 1, 0
        },
        std::array<char, buttonPixelDims * buttonPixelDims>{
            0, 1, 1, 1, 1, 1, 1, 1, 0,
            1, 0, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 1, 0, 0, 0, 0, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 0, 0, 0, 0, 1, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 0, 1,
            0, 1, 1, 1, 1, 1, 1, 1, 0
        },
        std::array<char, buttonPixelDims * buttonPixelDims>{
            0, 1, 1, 1, 1, 1, 1, 1, 0,
            1, 0, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 1, 0, 0, 0, 0, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 0, 0, 1, 0, 0, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 0, 0, 0, 0, 1, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 0, 1,
            0, 1, 1, 1, 1, 1, 1, 1, 0
        },
        std::array<char, buttonPixelDims * buttonPixelDims>{
            0, 1, 1, 1, 1, 1, 1, 1, 0,
            1, 0, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 1, 0, 0, 0, 1, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 1, 0, 0, 0, 1, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 0, 1,
            0, 1, 1, 1, 1, 1, 1, 1, 0
        },
        std::array<char, buttonPixelDims * buttonPixelDims>{
            0, 1, 1, 1, 1, 1, 1, 1, 0,
            1, 0, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 1, 0, 0, 0, 1, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 0, 0, 1, 0, 0, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 1, 0, 0, 0, 1, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 0, 1,
            0, 1, 1, 1, 1, 1, 1, 1, 0
        },
        std::array<char, buttonPixelDims * buttonPixelDims>{
            0, 1, 1, 1, 1, 1, 1, 1, 0,
            1, 0, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 1, 0, 0, 0, 1, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 1, 0, 0, 0, 1, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 1, 0, 0, 0, 1, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 0, 1,
            0, 1, 1, 1, 1, 1, 1, 1, 0
        }
    };

    const std::array<char, buttonPixelDims * buttonPixelDims> randomShape {
        0, 1, 1, 1, 1, 1, 1, 1, 0,
        1, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 1, 0, 0, 0, 1, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 1, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 1, 0, 0, 0, 1, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 1,
        0, 1, 1, 1, 1, 1, 1, 1, 0
    };

    const std::array<char, buttonPixelDims * buttonPixelDims> shiftUpShape {
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 1, 0, 0, 0, 0,
        0, 0, 0, 1, 0, 1, 0, 0, 0,
        0, 0, 1, 0, 0, 0, 1, 0, 0,
        0, 1, 0, 0, 0, 0, 0, 1, 0,
        1, 0, 0, 0, 0, 0, 0, 0, 1,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0

    };
    const std::array<char, buttonPixelDims * buttonPixelDims> shiftDownShape {
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 0, 0, 0, 0, 0, 0, 0, 1,
        0, 1, 0, 0, 0, 0, 0, 1, 0,
        0, 0, 1, 0, 0, 0, 1, 0, 0,
        0, 0, 0, 1, 0, 1, 0, 0, 0,
        0, 0, 0, 0, 1, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0
    };

};

#endif //MAIM_ARRAYASSIGNERBUTTON_H

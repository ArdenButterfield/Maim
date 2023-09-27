//
// Created by arden on 7/18/23.
//

#include "MaimLookAndFeel.h"

MaimLookAndFeel::MaimLookAndFeel() :
                                     main_font (juce::Typeface::createSystemTypefaceFor(BinaryData::bedsteadsemicondensed_otf, BinaryData::bedsteadsemicondensed_otfSize)),
                                     bold_font(juce::Typeface::createSystemTypefaceFor(BinaryData::bedsteadboldsemicondensed_otf, BinaryData::bedsteadboldsemicondensed_otfSize)),
                                     tooltip_font(juce::Typeface::createSystemTypefaceFor(BinaryData::GontserratRegular_ttf, BinaryData::GontserratRegular_ttfSize))
{
    // this->setColour(juce::ResizableWindow::backgroundColourId, PANEL_BACKGROUND_COLOR);
    setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0xffb3b2a8));
    setColour(juce::ComboBox::backgroundColourId, MaimColours::BEVEL_LIGHT);
    setColour(juce::ComboBox::textColourId, MaimColours::BEVEL_BLACK);
    setColour(juce::ComboBox::outlineColourId, MaimColours::BEVEL_DARK);
    setColour(juce::ComboBox::focusedOutlineColourId, MaimColours::BEVEL_BLACK);
    setColour(juce::ComboBox::arrowColourId, MaimColours::BEVEL_DARK);

    setColour(juce::PopupMenu::backgroundColourId, MaimColours::BEVEL_LIGHT);
    setColour(juce::PopupMenu::textColourId, MaimColours::BEVEL_DARK);
    setColour(juce::PopupMenu::headerTextColourId, MaimColours::BEVEL_BLACK);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, MaimColours::BEVEL_WHITE);
    setColour(juce::PopupMenu::highlightedTextColourId, MaimColours::BEVEL_BLACK);

    setColour(juce::Slider::textBoxTextColourId, MaimColours::BEVEL_DARK);
    const float transparent_alpha = 0;
    setColour(juce::Slider::textBoxOutlineColourId,
        MaimColours::PANEL_BACKGROUND_COLOR.withAlpha(transparent_alpha));
    setColour(juce::Slider::textBoxHighlightColourId, MaimColours::BEVEL_BLACK);
    setColour(juce::Slider::textBoxBackgroundColourId,
        MaimColours::PANEL_BACKGROUND_COLOR.withAlpha(transparent_alpha));
    setColour(juce::Label::textWhenEditingColourId, MaimColours::BEVEL_BLACK);


    setColour(juce::CaretComponent::caretColourId, juce::Colour(0xff2e2edb));

}

MaimLookAndFeel::~MaimLookAndFeel()
{
}

void MaimLookAndFeel::drawRotarySlider (juce::Graphics& g,
    int x,
    int y,
    int width,
    int height,
    float sliderPos,
    const float rotaryStartAngle,
    const float rotaryEndAngle,
    juce::Slider& s)
{
    // Based on https://docs.juce.com/master/tutorial_look_and_feel_customisation.html

    juce::Colour fill = juce::Colours::white;
    juce::Colour outline = juce::Colours::black;
    juce::Colour notch = juce::Colour(0xff2d2d46);
    bool focused = false;

    const auto radius = (float) juce::jmin(width / 2, height / 2) - 5;
    const auto inner_radius = focused ? radius - 2 : radius;
    const auto centerx = (float) x + (float) width  * 0.5f;
    const auto centery = (float) y + (float) height * 0.5f;


    if (focused) {
        const float shadow_tilt = 0.2;

        const float penumbra_rad = 0.2;

        juce::Path highlight;
        highlight.startNewSubPath(centerx, centery);
        highlight.addArc(centerx - radius,
            centery - radius,
            radius * 2,
            radius * 2,
            pi * (shadow_tilt + 1),
            pi * (shadow_tilt + 2));
        highlight.closeSubPath();

        g.setColour(fill);
        g.fillPath(highlight);

        juce::Path penumbra;
        penumbra.startNewSubPath(centerx, centery);
        penumbra.addArc(centerx - radius,
            centery - radius,
            radius * 2,
            radius * 2,
            pi * shadow_tilt - penumbra_rad,
            pi * (shadow_tilt + 1) + penumbra_rad);
        penumbra.closeSubPath();

        g.setColour(MaimColours::BEVEL_LIGHT);
        g.fillPath(penumbra);

        juce::Path shadow;
        shadow.startNewSubPath(centerx, centery);
        shadow.addArc(centerx - radius,
            centery - radius,
            radius * 2,
            radius * 2,
            pi * shadow_tilt + penumbra_rad,
            pi * (shadow_tilt + 1) - penumbra_rad);
        shadow.closeSubPath();

        g.setColour(MaimColours::BEVEL_DARK);
        g.fillPath(shadow);

    }

    g.setColour(fill);
    g.fillEllipse(centerx - inner_radius,
        centery - inner_radius,
        inner_radius * 2,
        inner_radius * 2);

    g.setColour(outline);
    g.drawEllipse(centerx - radius,centery - radius,radius*2,radius*2, 1);

    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    g.setColour (notch);


    juce::Path p;
    auto pointerLength = inner_radius;
    auto pointerThickness = 2.0f;
    p.addRectangle (-pointerThickness * 0.5f, -inner_radius, pointerThickness, pointerLength);
    p.applyTransform (juce::AffineTransform::rotation (angle).translated (centerx, centery));
    g.fillPath (p);

}

void MaimLookAndFeel::drawLinearSlider(juce::Graphics& g,
    int x,
    int y,
    int width,
    int height,
    float sliderPos,
    float minSliderPos,
    float maxSliderPos,
    const juce::Slider::SliderStyle style,
    juce::Slider& s
)
{
    bool focused = false;

    juce::Colour white, light, dark, black, thumb_bg;
    white = MaimColours::BEVEL_WHITE;
    light = MaimColours::BEVEL_LIGHT;
    dark = MaimColours::BEVEL_DARK;
    black = MaimColours::BEVEL_BLACK;
    thumb_bg = MaimColours::PANEL_BACKGROUND_COLOR;

    const int pixel_size = 2;
    const int tick_spacing = 10; // 11 ticks

    // Make bar and thumb
    juce::Rectangle<int> bar, thumb;

    const int extra_bar_length = pixel_size * 3;
    const int thumb_length = 5;
    const int thumb_width = 12;

    if (style == juce::Slider::SliderStyle(juce::Slider::LinearHorizontal)) {
        const int center_y = y + (height / 2);
        bar = juce::Rectangle<int>(x - extra_bar_length,
            center_y - pixel_size * 2,
            width + extra_bar_length * 2,
            pixel_size * 4);
        thumb = juce::Rectangle<int>(0,0,pixel_size * thumb_length, pixel_size * thumb_width).withCentre(juce::Point<int>(sliderPos, center_y));

        if (focused) {
            int tick_x;
            g.setColour(dark);
            for (int i = 0; i < tick_spacing + 1; ++i) {
                tick_x = x + width * i / tick_spacing;

                g.drawVerticalLine(tick_x, center_y - 10, center_y - 8);
                g.drawVerticalLine(tick_x, center_y + 8, center_y + 10);
            }
        }
    } else if (style == juce::Slider::SliderStyle(juce::Slider::LinearVertical)) {
        const int center_x = x + (width / 2);
        bar = juce::Rectangle<int>(center_x - pixel_size * 2,
            y - extra_bar_length,
            pixel_size * 4,
            height + extra_bar_length * 2);
        thumb = juce::Rectangle<int>(0,0,pixel_size * thumb_width, pixel_size * thumb_length).withCentre(juce::Point<int>(center_x, sliderPos));

        if (focused) {
            int tick_y;
            g.setColour(dark);
            for (int i = 0; i < tick_spacing + 1; ++i) {
                tick_y = y + height * i / tick_spacing;

                g.drawHorizontalLine(tick_y, center_x - 10, center_x - 8);
                g.drawHorizontalLine(tick_y, center_x + 8, center_x + 10);
            }
        }
    }

    g.setColour(white);
    g.fillRect(bar);
    bar = bar.withTrimmedBottom(pixel_size).withTrimmedRight(pixel_size);
    g.setColour(dark);
    g.fillRect(bar);
    bar = bar.withTrimmedTop(pixel_size).withTrimmedLeft(pixel_size);
    g.setColour(light);
    g.fillRect(bar);
    bar = bar.withTrimmedBottom(pixel_size).withTrimmedRight(pixel_size);
    g.setColour(black);
    g.fillRect(bar);

    g.setColour(black);
    g.fillRect(thumb);
    thumb = thumb.withTrimmedBottom(pixel_size).withTrimmedRight(pixel_size);
    g.setColour(white);
    g.fillRect(thumb);
    thumb = thumb.withTrimmedTop(pixel_size).withTrimmedLeft(pixel_size);
    g.setColour(dark);
    g.fillRect(thumb);
    thumb = thumb.withTrimmedBottom(pixel_size).withTrimmedRight(pixel_size);
    g.setColour(thumb_bg);
    g.fillRect(thumb);
    // Draw thumb

}

void MaimLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) {

    const auto bottomOptionSelected = button.getToggleState();

    const auto topRect = button.getLocalBounds().withHeight(button.getHeight() / 2);
    const auto bottomRect = topRect.withBottomY(button.getHeight());

    const auto activeRect = bottomOptionSelected ? bottomRect : topRect;
    const auto inactiveRect = bottomOptionSelected ? topRect : bottomRect;
    const auto fillColour = bottomOptionSelected ? MaimColours::SPLASH_COLOR_LIGHT : MaimColours::CONTRAST_COLOR_LIGHT;
    const auto topName = button.getName().upToFirstOccurrenceOf("|", false, false);
    const auto bottomName = button.getName().fromLastOccurrenceOf("|", false, false);




    g.setColour(fillColour);
    g.fillRect(activeRect);
    g.setColour(MaimColours::BEVEL_LIGHT);
    g.fillRect(inactiveRect);
    g.setColour(MaimColours::BEVEL_BLACK);
    g.drawRect(button.getLocalBounds(), 2);
    g.setFont(juce::Font(main_font).withHeight(15));
    g.setColour(bottomOptionSelected ? MaimColours::BEVEL_DARK : MaimColours::BEVEL_BLACK);
    g.drawText(topName, topRect, juce::Justification::centred, false);
    g.setColour(bottomOptionSelected ? MaimColours::BEVEL_BLACK : MaimColours::BEVEL_DARK);
    g.drawText(bottomName, bottomRect, juce::Justification::centred, false);
}

#if false
void MaimLookAndFeel::drawComboBox (juce::Graphics& g,
                                    int width,
                                    int height,
                                    const bool isButtonDown,
                                    int buttonX,
                                    int buttonY,
                                    int buttonW,
                                    int buttonH,
                                    juce::ComboBox& box)
{
    g.setColour(juce::Colours::orange);
    g.fillRect(buttonX, buttonY, buttonW, buttonH);
    g.setColour(juce::Colours::green);
    auto boxbounds = box.getScreenBounds();
    g.drawRect(boxbounds,2);
}
#endif

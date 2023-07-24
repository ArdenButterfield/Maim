#include "PluginProcessor.h"
#include "juce_core/juce_core.h"
#include "juce_gui_basics/juce_gui_basics.h"

class MyJUCEApp  : public juce::JUCEApplication
{
public:
    MyJUCEApp()  {}
    ~MyJUCEApp() {}

    void initialise (const juce::String& commandLine) override
    {
    }

    void shutdown() override
    {
    }

    const juce::String getApplicationName() override
    {
        return "Super JUCE-o-matic";
    }

    const juce::String getApplicationVersion() override
    {
        return "1.0";
    }

private:
    // std::unique_ptr<MyApplicationWindow> myMainWindow;
};

// this generates boilerplate code to launch our app class:
START_JUCE_APPLICATION (MyJUCEApp)
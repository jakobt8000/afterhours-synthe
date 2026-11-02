// Synthe · AFTERHOURS
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

namespace syui
{
class Knob : public juce::Component
{
public:
    Knob (juce::RangedAudioParameter& p, juce::String label, std::function<juce::String (float)> fmt, float size);
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;
private:
    juce::RangedAudioParameter& param;
    juce::ParameterAttachment attach;
    juce::String label;
    std::function<juce::String (float)> fmt;
    float size, value = 0, start = 0;
};

class Content;
class PresetList : public juce::Component
{
public:
    explicit PresetList (Content& c) : owner (c) {}
    void paint (juce::Graphics&) override;
    void mouseUp (const juce::MouseEvent&) override;
    std::vector<int> shown;
private:
    Content& owner;
};

class Content : public juce::Component, private juce::Timer
{
public:
    explicit Content (SynthProcessor&);
    void paint (juce::Graphics&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void refreshList();
    SynthProcessor& proc;
    static constexpr int W = 940, H = 504, LEFT = 200;
private:
    void timerCallback() override;
    juce::OwnedArray<Knob> knobs;
    PresetList list { *this };
    juce::Viewport view;
    int lastState = -1;
};
} // namespace syui

class SynthEditor : public juce::AudioProcessorEditor
{
public:
    explicit SynthEditor (SynthProcessor&);
    void resized() override;
    void paint (juce::Graphics& g) override { g.fillAll (juce::Colour (0xff505053)); }
private:
    syui::Content content;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SynthEditor)
};

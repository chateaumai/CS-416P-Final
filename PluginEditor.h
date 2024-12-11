#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class CustomKnob : public juce::Slider
{
public:
    CustomKnob()
    {
        setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        setRange(0.0f, 1.0f, 0.01f);
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
       
        // background color
        float value = getValue();
        if (value < 0.33f)
            g.setColour(juce::Colour(100, 200, 100));
        else if (value < 0.66f)
            g.setColour(juce::Colour(200, 200, 100));
        else
            g.setColour(juce::Colour(200, 100, 100));
            
        g.fillEllipse(bounds);

        // face
        g.setColour(juce::Colours::black);
        float centerX = bounds.getCentreX();
        float centerY = bounds.getCentreY();
        float size = bounds.getWidth() * 0.4f;

        g.fillEllipse(centerX - size, centerY - size * 0.5f, 10.0f, 10.0f);
        g.fillEllipse(centerX + size - 10.0f, centerY - size * 0.5f, 10.0f, 10.0f);

        // draw different mouths based on value
        juce::Path mouth;
        if (value < 0.33f) {
            // up curve
            mouth.startNewSubPath(centerX - size, centerY + size * 0.3f);
            mouth.quadraticTo(centerX, centerY + size, centerX + size, centerY + size * 0.3f);
        }
        else if (value < 0.66f) {
            // Straight mouth
            mouth.startNewSubPath(centerX - size, centerY + size * 0.5f);
            mouth.lineTo(centerX + size, centerY + size * 0.5f);
        }
        else {
            // sad mouth
            mouth.startNewSubPath(centerX - size, centerY + size * 0.7f);
            mouth.quadraticTo(centerX, centerY + size * 0.3f, centerX + size, centerY + size * 0.7f);
        }
        
        g.strokePath(mouth, juce::PathStrokeType(2.0f));

        // indicator
        g.setColour(juce::Colours::white);
        float angle = (getValue() - getMinimum()) / (getMaximum() - getMinimum()) * 2.0f * juce::MathConstants<float>::pi;
        float indicatorSize = bounds.getWidth() * 0.45f;
        float indicatorX = centerX + std::cos(angle - juce::MathConstants<float>::halfPi) * indicatorSize;
        float indicatorY = centerY + std::sin(angle - juce::MathConstants<float>::halfPi) * indicatorSize;
        g.drawLine(centerX, centerY, indicatorX, indicatorY, 3.0f);
    }
};

class CustomLabel : public juce::Label
{
public:
    CustomLabel()
    {
        setFont(juce::Font("Arial", 18.0f, juce::Font::bold));
        setColour(juce::Label::textColourId, juce::Colours::white);
        setJustificationType(juce::Justification::centred);
    }
};

class MaiMachineAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    MaiMachineAudioProcessorEditor(MaiMachineAudioProcessor&);
    ~MaiMachineAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    MaiMachineAudioProcessor& audioProcessor;
    CustomKnob saturationKnob;
    CustomLabel saturationLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> saturationAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MaiMachineAudioProcessorEditor)
};

#include "PluginProcessor.h"
#include "PluginEditor.h"

MaiMachineAudioProcessorEditor::MaiMachineAudioProcessorEditor(MaiMachineAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    addAndMakeVisible(saturationKnob);
    saturationLabel.setText("Saturation", juce::dontSendNotification);
    addAndMakeVisible(saturationLabel);

    saturationAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "saturation", saturationKnob);

    setSize(300, 400);
}

MaiMachineAudioProcessorEditor::~MaiMachineAudioProcessorEditor()
{
}

void MaiMachineAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(30, 30, 30));
}

void MaiMachineAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    auto labelHeight = 40;
    
    saturationLabel.setBounds(bounds.removeFromTop(labelHeight));
    saturationKnob.setBounds(bounds.reduced(50));
}

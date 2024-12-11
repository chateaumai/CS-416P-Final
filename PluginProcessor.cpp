/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

MaiMachineAudioProcessor::MaiMachineAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
    apvts(*this, nullptr, "Parameters", createParameters())
#endif
{
    // Remove the old parameter creation
    // addParameter(saturationAmount = new juce::AudioParameterFloat(...)); <- Remove this
}

juce::AudioProcessorValueTreeState::ParameterLayout MaiMachineAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "saturation",  // parameterID
        "Saturation", // parameter name
        0.0f,         // minimum value
        1.0f,         // maximum value
        0.0f));       // default value
        
    return { params.begin(), params.end() };
}

MaiMachineAudioProcessor::~MaiMachineAudioProcessor()
{
}

//==============================================================================
const juce::String MaiMachineAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MaiMachineAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MaiMachineAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MaiMachineAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MaiMachineAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MaiMachineAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MaiMachineAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MaiMachineAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MaiMachineAudioProcessor::getProgramName (int index)
{
    return {};
}

void MaiMachineAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void MaiMachineAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void MaiMachineAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MaiMachineAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void MaiMachineAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    float amount = apvts.getRawParameterValue("saturation")->load();

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        float* channelData = buffer.getWritePointer(channel);
        
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float clean = channelData[i];
            float processed = clean;
            
            if (amount < 0.33f) {
                // Light crackly distortion
                float sectionAmount = amount * 3.0f;
                float drive = sectionAmount * 2.0f + 1.0f;
                processed *= drive;
               
                // WITHOUT this it would play static all the time if an instance of the VST was up
                // even when audio was not playing
                if (std::abs(clean) > 0.01f) {  // Threshold for when to add crackle
                    
                    float random = static_cast<float>(rand()) / RAND_MAX;
                    float crackleThreshold = 0.95f;
                    
                    float noise = 0.0f;
                    if (random > crackleThreshold) {
                        noise = (static_cast<float>(rand()) / RAND_MAX * 4.0f - 1.0f);
                        noise *= noise * noise;
                    }
                    
                    static float lastNoise = 0.0f;
                    noise = 0.8f * noise + 0.2f * lastNoise;
                    lastNoise = noise;
                    
                    float crackleAmount = sectionAmount * 0.04f * std::abs(clean);
                    processed = processed + (noise * crackleAmount);
                }
                
                processed = std::tanh(processed * 0.6f);
                processed += (processed * processed * processed * 0.1f);
                processed *= (1.0f - (std::abs(processed) * 0.05f));
                processed = (processed * 0.5f + clean * 0.5f);
            }
            else if (amount < 0.66f) {
                // Medium compression distortion
                float sectionAmount = (amount - 0.33f) * 3.0f;
                float drive = 1.5f + (sectionAmount * 2.0f);
                processed *= drive;
                
                processed = std::tanh(processed * 0.7f);
                processed += std::tanh(processed * 1.2f) * 0.08f;
                
                float comp = 1.0f / (1.0f + std::abs(processed) * 0.2f);
                processed *= comp * 0.9f;
            }
            else {
                // Shoegaze
                float sectionAmount = (amount - 0.66f) * 3.0f;
                float drive = 1.5f + (sectionAmount * 6.0f);
                processed *= drive;
                
                processed = std::tanh(processed * 0.8f);
                
                // trying to add harmonics like popgen
                float modulation = std::sin(static_cast<float>(i) * 0.0002f) * 0.15f + 1.0f;
                processed += std::sin(processed * (2.0f + modulation)) * 0.2f;
                processed += std::sin(processed * (3.0f + modulation)) * 0.1f;
                
                float comp = 1.0f / (1.0f + std::abs(processed) * 0.25f);
                processed *= comp * 0.85f;
            }
            
            channelData[i] = processed;
        }
    }
}
//==============================================================================
bool MaiMachineAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MaiMachineAudioProcessor::createEditor()
{
    return new MaiMachineAudioProcessorEditor (*this);
}

//==============================================================================
void MaiMachineAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void MaiMachineAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MaiMachineAudioProcessor();
}

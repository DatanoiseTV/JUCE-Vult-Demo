/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
JuceVultTemplateAudioProcessor::JuceVultTemplateAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    
    // Initialize Vult DSP
    Synth1_process_init(synth);
    Synth1_default(synth);
    
    // Add parameters for Plugin host, as we are not providing our own GUI.
    addParameter (volume = new juce::AudioParameterFloat ("volume", // parameterID
                                                        "Volume", // parameter name
                                                        0.0f,   // minimum value
                                                        1.0f,   // maximum value
                                                        0.9f)); // default value
    
    addParameter (detune = new juce::AudioParameterFloat ("detune", // parameterID
                                                        "Detune", // parameter name
                                                        0.0f,   // minimum value
                                                        1.0f,   // maximum value
                                                        0.0f)); // default value
    
    addParameter (lfo_rate = new juce::AudioParameterFloat ("lfo_rate", // parameterID
                                                        "LFO Rate", // parameter name
                                                        0.0f,   // minimum value
                                                        1.0f,   // maximum value
                                                        0.0f)); // default value
    
    addParameter (lfo_amount = new juce::AudioParameterFloat ("lfo_amount", // parameterID
                                                        "LFO Amount", // parameter name
                                                        0.0f,   // minimum value
                                                        1.0f,   // maximum value
                                                        0.0f)); // default value
    
    

}

JuceVultTemplateAudioProcessor::~JuceVultTemplateAudioProcessor()
{
}

//==============================================================================
const juce::String JuceVultTemplateAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool JuceVultTemplateAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool JuceVultTemplateAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool JuceVultTemplateAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double JuceVultTemplateAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int JuceVultTemplateAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int JuceVultTemplateAudioProcessor::getCurrentProgram()
{
    return 0;
}

void JuceVultTemplateAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String JuceVultTemplateAudioProcessor::getProgramName (int index)
{
    return {};
}

void JuceVultTemplateAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void JuceVultTemplateAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void JuceVultTemplateAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool JuceVultTemplateAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
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



void JuceVultTemplateAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    
    
    juce::MidiBuffer processedMidi;
        int time;
        juce::MidiMessage m;
     
        for (juce::MidiBuffer::Iterator i (midiMessages); i.getNextEvent (m, time);)
        {
            if (m.isNoteOn())
            {
                if(m.getVelocity() > 0){
                    Synth1_noteOn(synth, m.getNoteNumber(), m.getVelocity(), m.getChannel());
                    std::cout << "Note On" << std::endl;
                } else {
                    Synth1_noteOff(synth, m.getNoteNumber(), m.getChannel());
                    std::cout << "Note Off" << std::endl;
                }
            }
            else if (m.isNoteOff())
            {
                Synth1_noteOff(synth, m.getNoteNumber(), m.getChannel());
            }
            else if (m.isAftertouch())
            {
            }
            else if (m.isPitchWheel())
            {
            }
#ifdef USE_MIDI_CC
            else if (m.isController())
            {
                Synth1_controlChange(synth, m.getControllerNumber(), m.getControllerValue(), m.getChannel());
            }
#endif
     
            processedMidi.addEvent (m, time);
        }
     
        midiMessages.swapWith (processedMidi);
     
#ifndef USE_MIDI_CC
    // ToDo: Instead of comparing, look up how to implement a change callback, which
    // is not clear in current JUCE documentation.
    float vol = *volume;
    if(old_volume != vol) {
        Synth1_controlChange(synth, 30, vol * 127.0, 0);
        old_volume = vol;
    }
    
    float det = *detune;
    if(old_detune != det){
        Synth1_controlChange(synth, 31, det * 127.0, 0);
        old_detune = det;
    }
    
    float lfor = *lfo_rate;
    if(old_lfo_rate != lfor){
        Synth1_controlChange(synth, 32, lfor * 127.0, 0);
        old_lfo_rate = lfor;
    }
    
    float lfoa = *lfo_amount;
    if(old_lfo_amount != lfor){
        Synth1_controlChange(synth, 33, lfoa * 127.0, 0);
        old_lfo_amount = lfoa;
    }
    
    
#endif
    
    
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    
    auto* inputData_l = buffer.getReadPointer(0);
    auto* inputData_r = buffer.getReadPointer(1);
    auto* channelData_l = buffer.getWritePointer(0);
    auto* channelData_r = buffer.getWritePointer(1);

        // ..do something to the data...
        for(int i = 0; i < buffer.getNumSamples(); i++){
            channelData_l[i] = Synth1_process(synth, inputData_l[i]);
            channelData_r[i] = Synth1_process(synth, inputData_r[i]);
        }
    
}

//==============================================================================
bool JuceVultTemplateAudioProcessor::hasEditor() const
{
    return false; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* JuceVultTemplateAudioProcessor::createEditor()
{
    //return new JuceVultTemplateAudioProcessorEditor (*this);
    return nullptr;
}

//==============================================================================
void JuceVultTemplateAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void JuceVultTemplateAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new JuceVultTemplateAudioProcessor();
}

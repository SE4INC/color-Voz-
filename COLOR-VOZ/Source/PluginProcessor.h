#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "ColorEngine.h"
#include "AnalysisWorker.h"
class ColorProcessor final : public juce::AudioProcessor,
 private juce::AudioProcessorValueTreeState::Listener {
public:
 ColorProcessor();~ColorProcessor() override;
 const juce::String getName() const override{return "CRIS VOX COLOR";}
 void prepareToPlay(double,int) override;void releaseResources() override{}
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
 bool isBusesLayoutSupported(const BusesLayout&) const override;
 bool acceptsMidi() const override{return false;}bool producesMidi() const override{return false;}
 double getTailLengthSeconds() const override{return .12;}
 bool hasEditor() const override{return true;}juce::AudioProcessorEditor* createEditor() override;
 int getNumPrograms() override{return int(cris::presets.size());}
 int getCurrentProgram() override{return currentProgram.load();}
 void setCurrentProgram(int) override;
 const juce::String getProgramName(int i) override{return cris::presets[size_t(juce::jlimit(0,getNumPrograms()-1,i))].name;}
 void changeProgramName(int,const juce::String&) override{}
 void getStateInformation(juce::MemoryBlock&) override;void setStateInformation(const void*,int) override;
 juce::AudioProcessorValueTreeState parameters;
 cris::AnalysisWorker analysis;
 cris::Values values() const;
 void apply(const cris::Values&);void preview(const cris::Values&);void stopPreview(){previewing=false;}
 bool isPreviewing() const{return previewing.load();}
 std::atomic<float> inputPeak{0},outputPeak{0},compensation{0};
private:
 static juce::AudioProcessorValueTreeState::ParameterLayout layout();
 void parameterChanged(const juce::String&,float) override{previewing=false;}
 std::array<std::atomic<float>*,7> raw{};
 std::array<std::atomic<float>,7> proposal{};
 std::atomic<float>* match=nullptr;std::atomic<float>* bypass=nullptr;
 std::atomic<bool> previewing{false};std::atomic<int> currentProgram{0};
 cris::ColorEngine engine;int chunkSize=512;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ColorProcessor)
};

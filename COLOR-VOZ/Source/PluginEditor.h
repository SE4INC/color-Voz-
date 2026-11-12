#pragma once
#include "PluginProcessor.h"
class ColorLook final: public juce::LookAndFeel_V4 {
public:
 ColorLook();
 void drawRotarySlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider&) override;
};
class ColorEditor final:public juce::AudioProcessorEditor,private juce::Timer {
public:
 explicit ColorEditor(ColorProcessor&);~ColorEditor() override;
 void paint(juce::Graphics&) override;void resized() override;
private:
 void timerCallback() override;void refreshProposal();
 ColorProcessor& proc;ColorLook look;
 std::array<juce::Slider,7> knobs;
 std::array<juce::Label,7> labels;
 std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>,7> attachments;
 juce::ComboBox preset,intention,intensity;
 juce::TextButton analyse{"ANALIZAR VOZ"},listen{"ESCUCHAR"},applyButton{"APLICAR"},discard{"DESCARTAR"};
 juce::ToggleButton bypass{"ORIGINAL"},match{"IGUALAR VOLUMEN"};
 std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment,matchAttachment;
 juce::Label status,explanation,proposed,modeLabel,intensityLabel;
 cris::AnalysisResult result;cris::Proposal proposal{cris::neutral,{}};
 juce::Rectangle<int> graph,panel;bool lastValid=false;int lastProgram=-1;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ColorEditor)
};

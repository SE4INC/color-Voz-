#pragma once
#include <juce_dsp/juce_dsp.h>
#include <array>
namespace cris {
// Body, presence, crunch, air, smooth, mix (0..1), output (dB).
using Values = std::array<float,7>;
inline constexpr Values neutral{0,0,0,0,0.45f,1,0};
inline constexpr const char* ids[]{"body","presence","crunch","air","smooth","mix","output"};
inline constexpr const char* names[]{"BODY","PRESENCE","CRUNCH","AIR","SMOOTH","MIX","OUTPUT"};
struct Preset { const char* name; Values values; };
inline constexpr std::array<Preset,6> presets{{
 {"NEUTRO",neutral}, {"MEDIOS DORADOS",{.28f,.32f,.20f,.12f,.50f,.75f,0}},
 {"CROCANTE SUAVE",{.10f,.35f,.45f,.20f,.60f,.65f,0}},
 {"AIRE SEDOSO",{.08f,.15f,.12f,.48f,.65f,.70f,0}},
 {"RAP CON MORDIDA",{.20f,.50f,.60f,.18f,.65f,.65f,0}},
 {"MELODICO CALIDO",{.38f,.18f,.15f,.15f,.50f,.70f,0}}
}};
class ColorEngine {
public:
 void prepare(double sampleRate,int maximumBlock,int channels);
 void reset();
 void set(const Values&,bool match,bool bypass);
 void process(juce::AudioBuffer<float>&,int offset,int count);
 int latency() const {return delay;}
 float compensationDb() const {return juce::Decibels::gainToDecibels(matchGain,-60.0f);}
private:
 struct Channel {float low=0,mid=0,dcIn=0,dcOut=0,envelope=0;};
 std::array<Channel,2> states{};
 std::array<juce::SmoothedValue<float>,7> controls;
 juce::SmoothedValue<float> bypassSmooth;
 std::unique_ptr<juce::dsp::Oversampling<float>> oversampling;
 juce::AudioBuffer<float> delta;
 std::array<std::array<float,256>,2> delayLine{};
 int delay=0,position=0,numChannels=2;
 float lowA=0,midA=0,dcA=0,attack=0,release=0;
 float energyA=0,matchA=0,matchGain=1;
 double inputEnergy=0,wetEnergy=0;
 bool doMatch=true;
};
}

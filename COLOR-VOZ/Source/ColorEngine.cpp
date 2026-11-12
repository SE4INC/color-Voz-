#include "ColorEngine.h"
#include <cmath>
namespace cris {
void ColorEngine::prepare(double sr,int maxBlock,int channels) {
 numChannels=juce::jlimit(1,2,channels);
 oversampling=std::make_unique<juce::dsp::Oversampling<float>>(numChannels,1,
   juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,true,true);
 oversampling->initProcessing(static_cast<size_t>(maxBlock));
 delay=static_cast<int>(std::lround(oversampling->getLatencyInSamples()));
 jassert(delay<256);
 delta.setSize(numChannels,maxBlock);
 auto pole=[sr](double hz){return float(std::exp(-juce::MathConstants<double>::twoPi*hz/(2*sr)));};
 lowA=pole(250);midA=pole(3500);dcA=pole(12);
 attack=float(std::exp(-1/(.001*2*sr)));release=float(std::exp(-1/(.070*2*sr)));
 energyA=float(std::exp(-1/(.5*sr)));matchA=float(std::exp(-1/(.4*sr)));
 for(auto& c:controls)c.reset(2*sr,.025);
 bypassSmooth.reset(sr,.010);reset();
}
void ColorEngine::reset() {
 states={};delayLine={};position=0;inputEnergy=wetEnergy=0;matchGain=1;
 if(oversampling)oversampling->reset();
 for(size_t i=0;i<controls.size();++i)controls[i].setCurrentAndTargetValue(neutral[i]);
 bypassSmooth.setCurrentAndTargetValue(0);
}
void ColorEngine::set(const Values& v,bool match,bool bypass) {
 for(size_t i=0;i<v.size();++i)controls[i].setTargetValue(i==6?
   juce::jlimit(-24.0f,12.0f,v[i]):juce::jlimit(0.0f,1.0f,v[i]));
 doMatch=match;bypassSmooth.setTargetValue(bypass?1.0f:0.0f);
}
void ColorEngine::process(juce::AudioBuffer<float>& b,int offset,int count) {
 for(int ch=0;ch<numChannels;++ch)delta.copyFrom(ch,0,b,ch,offset,count);
 const float mixStart=controls[5].getCurrentValue();
 const float outputStart=controls[6].getCurrentValue();
 juce::dsp::AudioBlock<float> block(delta);
 auto sub=block.getSubBlock(0,static_cast<size_t>(count));
 auto up=oversampling->processSamplesUp(sub);
 // Only the colour difference passes through the oversampler. At neutral the
 // output is exactly delayed dry, avoiding coloration from crossover phase.
 for(size_t i=0;i<up.getNumSamples();++i) {
  Values v{};for(size_t k=0;k<v.size();++k)v[k]=controls[k].getNextValue();
  const float body=v[0],pres=v[1],crunch=v[2],air=v[3],smooth=v[4];
  for(int ch=0;ch<numChannels;++ch) {
   auto& s=states[static_cast<size_t>(ch)];float x=up.getSample(ch,i);
   s.low=(1-lowA)*x+lowA*s.low;s.mid=(1-midA)*x+midA*s.mid;
   float lo=s.low,mid=s.mid-s.low,hi=x-s.mid;
   float amp=std::abs(hi),a=amp>s.envelope?attack:release;
   s.envelope=a*s.envelope+(1-a)*amp;
   float protection=1/(1+smooth*12*std::max(0.0f,s.envelope-.035f));
   auto colour=[](float x,float drive){return std::tanh(drive*x)/std::tanh(drive)-x;};
   float added=.28f*body*colour(lo,2.4f)+.55f*pres*mid
      +.50f*crunch*colour(mid,1.5f+6*crunch)
      +protection*(.38f*air*hi+.22f*air*colour(hi,3.5f+3*crunch));
   float dc=added-s.dcIn+dcA*s.dcOut;s.dcIn=added;s.dcOut=dc;
   up.setSample(ch,i,dc);
  }
 }
 oversampling->processSamplesDown(sub);
 // Mix/output ramp values are interpolated over the base-rate block below.
 const float mixEnd=controls[5].getCurrentValue();
 const float outputEnd=controls[6].getCurrentValue();
 for(int i=0;i<count;++i) {
  const float t=float(i+1)/float(count);
  const float mix=mixStart+t*(mixEnd-mixStart);
  const float outGain=juce::Decibels::decibelsToGain(outputStart+t*(outputEnd-outputStart));
  std::array<float,2> clean{},wet{};double inP=0,outP=0;
  for(int ch=0;ch<numChannels;++ch) {
   auto& line=delayLine[static_cast<size_t>(ch)];
   float x=b.getSample(ch,offset+i);line[static_cast<size_t>(position)]=x;
   int read=(position-delay+256)%256;clean[ch]=line[static_cast<size_t>(read)];
   wet[ch]=clean[ch]+delta.getSample(ch,i);
   inP+=clean[ch]*clean[ch];outP+=wet[ch]*wet[ch];
  }
  position=(position+1)%256;
  inputEnergy=energyA*inputEnergy+(1-energyA)*inP/numChannels;
  wetEnergy=energyA*wetEnergy+(1-energyA)*outP/numChannels;
  float target=1;
  if(doMatch && inputEnergy>1.e-8 && wetEnergy>1.e-8)
   target=juce::jlimit(.501187f,1.995262f,float(std::sqrt(inputEnergy/wetEnergy)));
  matchGain=matchA*matchGain+(1-matchA)*target;
  float bypass=bypassSmooth.getNextValue();
  for(int ch=0;ch<numChannels;++ch) {
   float y=clean[ch]+mix*(wet[ch]*matchGain-clean[ch]);
   b.setSample(ch,offset+i,(y+(clean[ch]-y)*bypass)*outGain);
  }
 }
}
}

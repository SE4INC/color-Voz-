#include "PluginProcessor.h"
#include "PluginEditor.h"
juce::AudioProcessorValueTreeState::ParameterLayout ColorProcessor::layout(){
 juce::AudioProcessorValueTreeState::ParameterLayout l;
 for(int i=0;i<7;++i)l.add(std::make_unique<juce::AudioParameterFloat>(
  juce::ParameterID{cris::ids[i],1},cris::names[i],
  juce::NormalisableRange<float>(i==6?-24.0f:0.0f,i==6?12.0f:1.0f,.001f),cris::neutral[size_t(i)]));
 l.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"match",1},"Level match",true));
 l.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"bypass",1},"Bypass",false));
 return l;
}
ColorProcessor::ColorProcessor():AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true)
 .withOutput("Output",juce::AudioChannelSet::stereo(),true)),parameters(*this,nullptr,"CRIS_COLOR_PARAMETERS",layout()) {
 for(size_t i=0;i<7;++i){raw[i]=parameters.getRawParameterValue(cris::ids[i]);proposal[i]=cris::neutral[i];parameters.addParameterListener(cris::ids[i],this);}
 match=parameters.getRawParameterValue("match");bypass=parameters.getRawParameterValue("bypass");
}
ColorProcessor::~ColorProcessor(){for(auto id:cris::ids)parameters.removeParameterListener(id,this);}
bool ColorProcessor::isBusesLayoutSupported(const BusesLayout& l)const{
 auto out=l.getMainOutputChannelSet();return (out==juce::AudioChannelSet::mono()||out==juce::AudioChannelSet::stereo())&&out==l.getMainInputChannelSet();
}
void ColorProcessor::prepareToPlay(double sr,int maxBlock){
 chunkSize=juce::jlimit(32,8192,maxBlock);engine.prepare(sr,chunkSize,getTotalNumInputChannels());
 setLatencySamples(engine.latency());analysis.prepare(sr);previewing=false;
 inputPeak=outputPeak=0;
}
cris::Values ColorProcessor::values()const{cris::Values v;for(size_t i=0;i<7;++i)v[i]=raw[i]->load();return v;}
void ColorProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&){
 juce::ScopedNoDenormals guard;auto v=values();
 if(previewing.load())for(size_t i=0;i<7;++i)v[i]=proposal[i].load();
 engine.set(v,match->load()>.5f,bypass->load()>.5f);
 float in=0,out=0;
 for(int ch=0;ch<b.getNumChannels();++ch)in=std::max(in,b.getMagnitude(ch,0,b.getNumSamples()));
 if(b.getNumChannels()>0)analysis.push(b.getReadPointer(0),b.getNumSamples());
 for(int start=0;start<b.getNumSamples();start+=chunkSize)engine.process(b,start,std::min(chunkSize,b.getNumSamples()-start));
 for(int ch=0;ch<b.getNumChannels();++ch)out=std::max(out,b.getMagnitude(ch,0,b.getNumSamples()));
 inputPeak=in;outputPeak=out;compensation=engine.compensationDb();
}
void ColorProcessor::apply(const cris::Values& v){
 previewing=false;
 for(size_t i=0;i<7;++i){auto* p=parameters.getParameter(cris::ids[i]);p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1(v[i]));p->endChangeGesture();}
}
void ColorProcessor::preview(const cris::Values& v){previewing=false;for(size_t i=0;i<7;++i)proposal[i]=v[i];previewing=true;}
void ColorProcessor::setCurrentProgram(int i){i=juce::jlimit(0,getNumPrograms()-1,i);currentProgram=i;apply(cris::presets[size_t(i)].values);}
void ColorProcessor::getStateInformation(juce::MemoryBlock& b){auto state=parameters.copyState();state.setProperty("program",currentProgram.load(),nullptr);auto xml=state.createXml();copyXmlToBinary(*xml,b);}
void ColorProcessor::setStateInformation(const void* data,int size){
 if(auto xml=getXmlFromBinary(data,size))if(xml->hasTagName(parameters.state.getType())){
  auto tree=juce::ValueTree::fromXml(*xml);parameters.replaceState(tree);
  currentProgram=juce::jlimit(0,getNumPrograms()-1,int(tree.getProperty("program",0)));previewing=false;
 }
}
juce::AudioProcessorEditor* ColorProcessor::createEditor(){return new ColorEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new ColorProcessor;}

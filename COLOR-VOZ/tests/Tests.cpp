#include "PluginProcessor.h"
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <new>
#if defined(_MSC_VER)
#include <malloc.h>
#endif
namespace probe {thread_local bool enabled=false;thread_local int count=0;}
void* operator new(std::size_t n){if(probe::enabled)++probe::count;if(auto* p=std::malloc(n?n:1))return p;throw std::bad_alloc();}
void* operator new[](std::size_t n){return ::operator new(n);}
void operator delete(void* p)noexcept{std::free(p);}void operator delete[](void* p)noexcept{std::free(p);}
void operator delete(void* p,std::size_t)noexcept{std::free(p);}void operator delete[](void* p,std::size_t)noexcept{std::free(p);}
void* operator new(std::size_t n,std::align_val_t a){if(probe::enabled)++probe::count;auto alignment=static_cast<size_t>(a);
#if defined(_MSC_VER)
 void* p=_aligned_malloc(n?n:1,alignment);
#else
 void* p=std::aligned_alloc(alignment,((std::max<size_t>(1,n)+alignment-1)/alignment)*alignment);
#endif
 if(p)return p;throw std::bad_alloc();}
void* operator new[](std::size_t n,std::align_val_t a){return ::operator new(n,a);}
void operator delete(void* p,std::align_val_t)noexcept{
#if defined(_MSC_VER)
 _aligned_free(p);
#else
 std::free(p);
#endif
}
void operator delete[](void* p,std::align_val_t a)noexcept{::operator delete(p,a);}
void operator delete(void* p,std::size_t,std::align_val_t a)noexcept{::operator delete(p,a);}
void operator delete[](void* p,std::size_t,std::align_val_t a)noexcept{::operator delete(p,a);}
void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char** argv){try{
 juce::ScopedJuceInitialiser_GUI juce;
 for(double sr:{44100.0,48000.0,96000.0})for(int channels:{1,2})for(int block:{64,128,2048}){
  cris::ColorEngine engine;engine.prepare(sr,block,channels);engine.set(cris::neutral,false,false);
  juce::AudioBuffer<float> b(channels,block);long long t=0;
  for(int repeat=0;repeat<20;++repeat){for(int c=0;c<channels;++c)for(int i=0;i<block;++i)b.setSample(c,i,.2f*std::sin(float((t+i)*1000*juce::MathConstants<double>::twoPi/sr)));
   probe::count=0;probe::enabled=true;engine.process(b,0,block);probe::enabled=false;check(probe::count==0,"Audio allocation in engine");
   for(int c=0;c<channels;++c)for(int i=0;i<block;++i){double expected=t+i>=engine.latency()?.2f*std::sin(float((t+i-engine.latency())*1000*juce::MathConstants<double>::twoPi/sr)):0;check(std::abs(b.getSample(c,i)-expected)<1.e-6,"Neutral must null with delayed original");}t+=block;
  }
  engine.set(cris::presets[4].values,true,false);auto start=std::chrono::steady_clock::now();
  for(int repeat=0;repeat<int(sr/block);++repeat){for(int c=0;c<channels;++c)for(int i=0;i<block;++i)b.setSample(c,i,.25f*std::sin(float((t+i)*1100*juce::MathConstants<double>::twoPi/sr)));
   probe::enabled=true;engine.process(b,0,block);probe::enabled=false;
   for(int c=0;c<channels;++c)for(int i=0;i<block;++i)check(std::isfinite(b.getSample(c,i))&&std::abs(b.getSample(c,i))<4,"Unstable output");t+=block;
  }
  check(probe::count==0,"Allocation under saturation");
  auto ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
  std::cout<<"One second audio: "<<sr<<" Hz / "<<channels<<" channels / "<<block<<" samples = "<<ms<<" ms\n";
 }
 // MIX=0 and ORIGINAL must retain the dry signal after parameter settling.
 for(bool bypass:{false,true}){
  cris::ColorEngine e;e.prepare(48000,256,2);auto settings=cris::presets[4].values;
  if(!bypass)settings[5]=0;e.set(settings,true,bypass);
  juce::AudioBuffer<float> block(2,256);int t=0;
  for(int n=0;n<50;++n){for(int c=0;c<2;++c)for(int i=0;i<256;++i)block.setSample(c,i,.1f*std::sin(float((t+i)*.1)));
   e.process(block,0,256);
   if(n>20)for(int i=0;i<256;++i)check(std::abs(block.getSample(0,i)-.1f*std::sin(float((t+i-e.latency())*.1)))<1.e-5,"Dry mix/bypass alignment");t+=256;
  }
 }
 // The nonlinear stage must actually generate new harmonic energy.
 cris::ColorEngine engine;engine.prepare(48000,256,1);cris::Values v=cris::neutral;v[2]=.8f;engine.set(v,false,false);
 juce::AudioBuffer<float> tone(1,256);double real=0,imag=0;int time=0;
 for(int n=0;n<400;++n){for(int i=0;i<256;++i)tone.setSample(0,i,.3f*std::sin(float(juce::MathConstants<double>::twoPi*1000*(time+i)/48000)));
  engine.process(tone,0,256);if(n>=25)for(int i=0;i<256;++i){double phase=juce::MathConstants<double>::twoPi*3000*(time+i)/48000;real+=tone.getSample(0,i)*std::cos(phase);imag+=tone.getSample(0,i)*std::sin(phase);}time+=256;}
 double harmonic=2*std::hypot(real,imag)/(375*256);check(harmonic>.001,"No measurable saturation harmonics");std::cout<<"Third harmonic amplitude: "<<harmonic<<"\n";
 ColorProcessor p;p.prepareToPlay(48000,2048);juce::AudioBuffer<float> b(2,2048);juce::MidiBuffer midi;
 p.setCurrentProgram(4);juce::MemoryBlock state;p.getStateInformation(state);ColorProcessor restored;restored.setStateInformation(state.getData(),int(state.getSize()));
 check(restored.values()==p.values(),"State roundtrip");check(restored.getCurrentProgram()==4,"Preset roundtrip");
 auto before=p.values();p.preview(cris::presets[2].values);check(p.values()==before,"Preview changed saved parameters");p.stopPreview();
 p.analysis.request();juce::Thread::sleep(30);
 for(int n=0;n<100;++n){b.clear();probe::count=0;probe::enabled=true;p.processBlock(b,midi);probe::enabled=false;check(probe::count==0,"Processor allocates while analysing");}
 p.analysis.cancel();
 // Real vocal fixture; this proves processing, not perceptual improvement.
 juce::AudioFormatManager formats;formats.registerBasicFormats();std::unique_ptr<juce::AudioFormatReader> reader(formats.createReaderFor(juce::File(CRIS_TEST_VOCAL)));
 check(bool(reader),"Missing vocal fixture");juce::AudioBuffer<float> voice(int(reader->numChannels),int(reader->lengthInSamples));reader->read(&voice,0,voice.getNumSamples(),0,true,true);
 p.prepareToPlay(reader->sampleRate,256);p.setCurrentProgram(2);b.setSize(2,256);double difference=0;
 p.analysis.request();juce::Thread::sleep(30);
 int total=int(reader->sampleRate*4.3);
 for(int t=0;t<total;t+=256){for(int i=0;i<256;++i)for(int c=0;c<2;++c)b.setSample(c,i,voice.getSample(0,(t+i)%voice.getNumSamples()));
  auto original=b.getSample(0,128);p.processBlock(b,midi);difference+=std::abs(b.getSample(0,128)-original);juce::Thread::sleep(6);}
 for(int i=0;i<50&&p.analysis.isCapturing();++i)juce::Thread::sleep(20);
 auto result=p.analysis.snapshot();check(difference>.1,"Real voice unchanged");check(result.valid,"Analysis failed on real vocal fixture");
 auto suggestion=cris::recommend(result,1,1);check(suggestion.values[2]>0,"Assistant returned no colour");p.apply(suggestion.values);check(std::abs(p.values()[2]-suggestion.values[2])<.002f,"Apply recommendation");
 cris::AnalysisResult silence;check(cris::recommend(silence,0,0).values==cris::neutral,"Silence recommendation");
 std::cout<<"Vocal analysis: "<<result.rmsDb<<" dBFS, crest "<<result.crestDb<<" dB\n";
 if(argc>1){std::unique_ptr<juce::AudioProcessorEditor> editor(p.createEditor());editor->setVisible(true);editor->setSize(1220,760);
  auto image=editor->createComponentSnapshot(editor->getLocalBounds());juce::File file=juce::File::getCurrentWorkingDirectory().getChildFile(argv[1]);file.deleteFile();auto out=file.createOutputStream();check(bool(out),"Screenshot output");juce::PNGImageFormat png;png.writeImageToStream(image,*out);out.reset();
  editor->setSize(1120,740);auto small=editor->createComponentSnapshot(editor->getLocalBounds());
  auto smallFile=file.getSiblingFile("interface-small.png");smallFile.deleteFile();auto smallOut=smallFile.createOutputStream();png.writeImageToStream(small,*smallOut);
 }
 std::cout<<"PASS: DSP, allocation, real voice, assistant, presets and state.\n";return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<"\n";return 1;}}

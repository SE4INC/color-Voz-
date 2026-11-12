#include "AnalysisWorker.h"
#include <cmath>
namespace cris {
Proposal recommend(const AnalysisResult& a,int intention,int intensity) {
 if(!a.valid)return {neutral,"Analiza una toma con suficiente voz antes de aplicar."};
 float strength=std::array<float,3>{.60f,1.0f,1.35f}[size_t(juce::jlimit(0,2,intensity))];
 Values v=intention==0?Values{.35f,.24f,.20f,.12f,.45f,.75f,0}:
          intention==1?Values{.12f,.36f,.48f,.24f,.60f,.70f,0}:
                       Values{.08f,.20f,.18f,.48f,.60f,.70f,0};
 const bool bright=a.powerDb[3]-a.powerDb[2]>-5;
 const bool heavy=a.powerDb[0]-a.powerDb[1]>12;
 if(bright){v[3]*=.35f;v[4]=.85f;}
 if(heavy)v[0]*=.40f;
 if(a.rmsDb>-12){v[2]*=.6f;v[4]=std::max(v[4],.75f);}
 for(int i=0;i<4;++i)v[size_t(i)]=juce::jlimit(0.0f,.85f,v[size_t(i)]*strength);
 juce::String why=bright?"Ya hay bastante energia alta: AIR moderado y mayor proteccion.":
   "Propuesta de textura en medios y apertura gradual de agudos.";
 if(heavy)why+=" BODY reducido para conservar claridad.";
 if(a.rmsDb>-12)why+=" Entrada fuerte: menos CRUNCH.";
 why+=" Compara con tu instrumental; no es un diagnostico obligatorio.";
 return {v,why};
}
AnalysisWorker::AnalysisWorker():Thread("CRIS Color analysis"){startThread(juce::Thread::Priority::low);}
AnalysisWorker::~AnalysisWorker(){stopThread(3000);}
void AnalysisWorker::prepare(double sr){capturing=false;armed=false;sampleRate=sr;requestId.fetch_add(1);}
void AnalysisWorker::request(){capturing=false;armed=true;requestId.fetch_add(1);}
void AnalysisWorker::cancel(){capturing=false;armed=false;requestId.fetch_add(1);}
void AnalysisWorker::push(const float* x,int n) noexcept {
 if(!capturing.load(std::memory_order_relaxed))return;
 int a,b,c,d;fifo.prepareToWrite(n,a,b,c,d);
 for(int i=0;i<b;++i)queue[size_t(a+i)].x=x[i];
 for(int i=0;i<d;++i)queue[size_t(c+i)].x=x[b+i];
 fifo.finishedWrite(b+d);if(b+d<n)dropped.fetch_add(1,std::memory_order_relaxed);
}
void AnalysisWorker::publish(const AnalysisResult& r){std::lock_guard<std::mutex> lock(mutex);published=r;}
AnalysisResult AnalysisWorker::snapshot(){std::lock_guard<std::mutex> lock(mutex);return published;}
void AnalysisWorker::run(){
 juce::dsp::FFT fft(11);std::array<float,4096> data{};std::array<float,2048> window{};
 for(size_t i=0;i<window.size();++i)window[i]=.5f-.5f*std::cos(juce::MathConstants<float>::twoPi*float(i)/2047);
 unsigned seen=0,dropStart=0;int fill=0;long long received=0,active=0;
 double sum=0,peak=0,start=0,sr=48000;std::array<double,4> bandSum{};int frames=0;
 AnalysisResult result;
 while(!threadShouldExit()){
  auto req=requestId.load();
  if(req!=seen){
   seen=req;int ready=fifo.getNumReady();fifo.finishedRead(ready);
   fill=0;received=active=0;sum=peak=0;frames=0;bandSum={};result=AnalysisResult{};
   sr=sampleRate.load();start=juce::Time::getMillisecondCounterHiRes();dropStart=dropped.load();
   // prepare/cancel requests stay idle; request() sets a separate arm flag below.
   result.status="Reproduce voz aislada durante al menos 4 segundos.";
   capturing.store(armed.exchange(false));publish(result);
  }
  if(capturing.load()){
   int a,b,c,d;fifo.prepareToRead(std::min(4096,fifo.getNumReady()),a,b,c,d);
   auto consume=[&](int index,int count){for(int k=0;k<count;++k){
    float x=queue[size_t(index+k)].x;data[size_t(fill++)]=x;++received;
    if(fill==2048){
     double e=0,pk=0;for(int i=0;i<2048;++i){e+=double(data[i])*data[i];pk=std::max(pk,double(std::abs(data[i])));}
     if(e/2048>1.e-5){
      active+=2048;sum+=e;peak=std::max(peak,pk);++frames;
      for(int i=0;i<2048;++i)data[i]*=window[i];
      fft.performFrequencyOnlyForwardTransform(data.data());
      std::array<double,4> energy{};std::array<int,4> bins{};
      for(int kbin=1;kbin<1024;++kbin){double hz=kbin*sr/2048;
       int band=hz<250?0:hz<2000?1:hz<6000?2:3;
       if(hz>=80 && hz<16000){energy[size_t(band)]+=double(data[kbin])*data[kbin];++bins[size_t(band)];}
      }
      for(size_t j=0;j<4;++j)bandSum[j]+=energy[j]/std::max(1,bins[j]);
      for(size_t j=0;j<128;++j){double hz=30*std::pow(600.0,double(j)/127);int bin=juce::jlimit(1,1023,int(hz*2048/sr));
       result.spectrum[j]=juce::Decibels::gainToDecibels(data[size_t(bin)]/512,-100.0f);}
     }
     fill=0;
    }
   }};
   consume(a,b);consume(c,d);fifo.finishedRead(b+d);
   result.progress=juce::jlimit(0.0f,1.0f,float(received/(sr*4)));
   if(received>=sr*4 || juce::Time::getMillisecondCounterHiRes()-start>15000){
    capturing=false;result.valid=active>=sr && dropped.load()==dropStart;
    if(result.valid){
     result.rmsDb=float(10*std::log10(std::max(1.e-12,sum/active)));
     result.crestDb=float(20*std::log10(std::max(1.e-9,peak)))-result.rmsDb;
     for(size_t j=0;j<4;++j)result.powerDb[j]=float(10*std::log10(std::max(1.e-12,bandSum[j]/std::max(1,frames))));
     result.status="Analisis listo. Elige caracter e intensidad y escucha.";
    }else result.status=dropped.load()!=dropStart?"Captura incompleta. Repite el analisis.":"Falta voz suficiente. Reproduce una toma y vuelve a analizar.";
   }
   publish(result);
  }
  wait(12);
 }
}
}

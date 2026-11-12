#pragma once
#include "ColorEngine.h"
#include <mutex>
namespace cris {
struct AnalysisResult {
 AnalysisResult(){spectrum.fill(-100.0f);}
 bool valid=false; float progress=0, rmsDb=-100, crestDb=0;
 std::array<float,4> powerDb{{-100,-100,-100,-100}};
 std::array<float,128> spectrum{};
 juce::String status="Reproduce tu voz y pulsa ANALIZAR.";
};
struct Proposal { Values values;juce::String explanation; };
Proposal recommend(const AnalysisResult&,int intention,int intensity);
class AnalysisWorker : public juce::Thread {
public:
 AnalysisWorker();~AnalysisWorker() override;
 void prepare(double sr);
 void push(const float*,int) noexcept;
 void request();void cancel();
 bool isCapturing() const {return capturing.load();}
 AnalysisResult snapshot();
 void run() override;
private:
 struct Frame {float x;};
 juce::AbstractFifo fifo{65536};std::array<Frame,65536> queue{};
 std::atomic<bool> armed{false},capturing{false};std::atomic<unsigned> requestId{0},dropped{0};
 std::atomic<double> sampleRate{48000};
 std::mutex mutex;AnalysisResult published;
 void publish(const AnalysisResult&);
};
}

#include "PluginEditor.h"
namespace {const juce::Colour gold(0xffd8b56b),bg(0xff101115),card(0xff1b1d23),ink(0xffeae8e2),muted(0xff92969f);}
ColorLook::ColorLook(){
 setColour(juce::Slider::textBoxTextColourId,ink);setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
 setColour(juce::Slider::textBoxBackgroundColourId,bg);
 setColour(juce::ComboBox::backgroundColourId,bg);setColour(juce::ComboBox::textColourId,ink);
 setColour(juce::ComboBox::outlineColourId,juce::Colour(0xff41434a));
 setColour(juce::PopupMenu::backgroundColourId,card);setColour(juce::PopupMenu::textColourId,ink);
 setColour(juce::TextButton::buttonColourId,juce::Colour(0xff34302a));setColour(juce::TextButton::textColourOffId,gold);
 setColour(juce::ToggleButton::textColourId,ink);setColour(juce::ToggleButton::tickColourId,gold);
}
void ColorLook::drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float v,float start,float end,juce::Slider&){
 auto r=juce::Rectangle<float>(float(x),float(y),float(w),float(h)).reduced(9);float radius=std::min(r.getWidth(),r.getHeight())*.5f;
 auto centre=r.getCentre();juce::Path base,active;
 base.addCentredArc(centre.x,centre.y,radius,radius,0,start,end,true);
 active.addCentredArc(centre.x,centre.y,radius,radius,0,start,start+v*(end-start),true);
 g.setColour(juce::Colour(0xff353840));g.strokePath(base,juce::PathStrokeType(4));
 g.setColour(gold);g.strokePath(active,juce::PathStrokeType(4));
 g.setGradientFill(juce::ColourGradient(juce::Colour(0xff3b3e46),centre.x,centre.y-radius,juce::Colour(0xff181a20),centre.x,centre.y+radius,false));
 g.fillEllipse(centre.x-radius+7,centre.y-radius+7,2*radius-14,2*radius-14);
 float a=start+v*(end-start);g.setColour(ink);
 g.drawLine(centre.x+std::sin(a)*(radius*.48f),centre.y-std::cos(a)*(radius*.48f),centre.x+std::sin(a)*(radius*.76f),centre.y-std::cos(a)*(radius*.76f),2.5f);
}
ColorEditor::ColorEditor(ColorProcessor& p):AudioProcessorEditor(p),proc(p){
 setLookAndFeel(&look);setSize(1220,760);setResizable(true,true);setResizeLimits(1120,740,1600,1000);
 for(size_t i=0;i<7;++i){
  auto& s=knobs[i];s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);s.setTextBoxStyle(juce::Slider::TextBoxBelow,false,85,24);
  s.setDoubleClickReturnValue(true,cris::neutral[i]);
  s.textFromValueFunction=[i](double v){return i==6?juce::String(v,1)+" dB":juce::String(int(std::lround(v*100)))+" %";};
  s.valueFromTextFunction=[i](const juce::String& t){return t.getDoubleValue()/(i==6?1.0:100.0);};
  addAndMakeVisible(s);labels[i].setText(cris::names[i],juce::dontSendNotification);labels[i].setJustificationType(juce::Justification::centred);labels[i].setColour(juce::Label::textColourId,ink);addAndMakeVisible(labels[i]);
  attachments[i]=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters,cris::ids[i],s);
  s.textFromValueFunction=[i](double v){return i==6?juce::String(v,1)+" dB":juce::String(int(std::lround(v*100)))+" %";};
  s.valueFromTextFunction=[i](const juce::String& t){return t.getDoubleValue()/(i==6?1.0:100.0);};s.updateText();
 }
 knobs[0].setTooltip("Cuerpo y armonicos suaves por debajo de 250 Hz.");
 knobs[1].setTooltip("Presencia en la zona media, sin desplazar el tono.");
 knobs[2].setTooltip("Saturacion de medios: empieza con cantidades pequenas.");
 knobs[3].setTooltip("Brillo y armonicos altos.");knobs[4].setTooltip("Reduce el color anadido cuando hay mucha energia aguda. No sustituye un de-esser.");
 for(int i=0;i<p.getNumPrograms();++i)preset.addItem(p.getProgramName(i),i+1);
 preset.setSelectedId(p.getCurrentProgram()+1,juce::dontSendNotification);preset.onChange=[this]{proc.setCurrentProgram(preset.getSelectedId()-1);};
 intention.addItemList({"Calida","Crocante","Brillante"},1);intention.setSelectedId(2);
 intensity.addItemList({"Sutil","Equilibrada","Intensa"},1);intensity.setSelectedId(2);
 intention.onChange=[this]{proc.stopPreview();refreshProposal();};intensity.onChange=intention.onChange;
 for(auto* c:{&preset,&intention,&intensity})addAndMakeVisible(*c);
 for(auto* b:{&analyse,&listen,&applyButton,&discard})addAndMakeVisible(*b);
 addAndMakeVisible(bypass);addAndMakeVisible(match);
 bypassAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.parameters,"bypass",bypass);
 matchAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.parameters,"match",match);
 for(auto* l:{&status,&explanation,&proposed,&modeLabel,&intensityLabel}){addAndMakeVisible(*l);l->setColour(juce::Label::textColourId,muted);l->setJustificationType(juce::Justification::topLeft);}
 modeLabel.setText("CARACTER",juce::dontSendNotification);intensityLabel.setText("INTENSIDAD",juce::dontSendNotification);
 proposed.setColour(juce::Label::textColourId,gold);
 analyse.onClick=[this]{proc.stopPreview();lastValid=false;result.valid=false;refreshProposal();proc.analysis.request();};
 listen.onClick=[this]{if(proc.isPreviewing())proc.stopPreview();else if(result.valid){proposal.values[6]=proc.values()[6];proc.preview(proposal.values);}};
 applyButton.onClick=[this]{if(result.valid){proposal.values[6]=proc.values()[6];proc.apply(proposal.values);status.setText("Propuesta aplicada. Ajusta a tu gusto.",juce::dontSendNotification);}};
 discard.onClick=[this]{proc.stopPreview();proc.analysis.cancel();result.valid=false;lastValid=false;refreshProposal();};
 result=proc.analysis.snapshot();status.setText(result.status,juce::dontSendNotification);refreshProposal();startTimerHz(20);
}
ColorEditor::~ColorEditor(){stopTimer();proc.stopPreview();setLookAndFeel(nullptr);}
void ColorEditor::refreshProposal(){
 proposal=cris::recommend(result,intention.getSelectedId()-1,intensity.getSelectedId()-1);
 // Assistant changes colour only, preserving the user's output level.
 proposal.values[6]=proc.values()[6];
 explanation.setText(proposal.explanation,juce::dontSendNotification);
 juce::String text;
 if(result.valid)for(int i=0;i<6;++i)text+=juce::String(cris::names[i])+"  "+juce::String(int(proposal.values[size_t(i)]*100))+" %"+(i%2==0?"     ":"\n");
 proposed.setText(text,juce::dontSendNotification);
 listen.setEnabled(result.valid);applyButton.setEnabled(result.valid);discard.setEnabled(result.valid);
}
void ColorEditor::timerCallback(){
 auto next=proc.analysis.snapshot();bool changed=next.valid!=lastValid;
 result=next;lastValid=next.valid;if(changed)refreshProposal();
 status.setText(result.status,juce::dontSendNotification);
 listen.setButtonText(proc.isPreviewing()?"DETENER":"ESCUCHAR");
 analyse.setEnabled(!proc.analysis.isCapturing());
 if(proc.getCurrentProgram()!=lastProgram){lastProgram=proc.getCurrentProgram();preset.setSelectedId(lastProgram+1,juce::dontSendNotification);}
 repaint();
}
void ColorEditor::resized(){
 int w=getWidth(),h=getHeight(),right=328,left=w-right-72;
 preset.setBounds(w-510,29,270,34);bypass.setBounds(w-216,30,180,32);
 graph={28,144,left,300+(h-760)/2};panel={w-right-28,110,right,h-146};
 int knobY=graph.getBottom()+58,kw=left/7;
 for(int i=0;i<7;++i){labels[size_t(i)].setBounds(28+i*kw,knobY,kw,22);knobs[size_t(i)].setBounds(28+i*kw,knobY+24,kw,125);}
 match.setBounds(36,h-63,200,30);
 int x=panel.getX()+18,pw=panel.getWidth()-36,y=panel.getY()+61;
 modeLabel.setBounds(x,y,pw,20);intention.setBounds(x,y+24,pw,32);
 intensityLabel.setBounds(x,y+68,pw,20);intensity.setBounds(x,y+92,pw,32);
 analyse.setBounds(x,y+140,pw,38);status.setBounds(x,y+190,pw,58);
 proposed.setBounds(x,y+260,pw,66);explanation.setBounds(x,y+334,pw,108);
 listen.setBounds(x,panel.getBottom()-90,(pw-8)/2,34);applyButton.setBounds(x+(pw+8)/2,panel.getBottom()-90,(pw-8)/2,34);
 discard.setBounds(x,panel.getBottom()-48,pw,28);
}
void ColorEditor::paint(juce::Graphics& g){
 g.fillAll(bg);g.setColour(gold);g.setFont(juce::FontOptions(27,juce::Font::bold));g.drawText("CRIS VOX COLOR",30,22,400,42,juce::Justification::centredLeft);
 g.setFont(juce::FontOptions(13));g.setColour(muted);g.drawText("VOCAL HARMONICS  /  1.0",32,66,400,22,juce::Justification::centredLeft);
 g.setColour(card);g.fillRoundedRectangle(panel.toFloat(),14);g.fillRoundedRectangle(graph.toFloat(),12);
 g.setColour(ink);g.setFont(juce::FontOptions(18,juce::Font::bold));g.drawText("ASISTENTE DE COLOR",panel.reduced(18).withHeight(30),juce::Justification::centredLeft);
 g.setFont(juce::FontOptions(13));g.setColour(muted);g.drawText("ESPECTRO CAPTURADO  /  ENTRADA MONO O CANAL L",graph.getX(),graph.getY()-29,graph.getWidth(),24,juce::Justification::centredLeft);
 auto plot=graph.toFloat().reduced(22,28);
 g.setColour(juce::Colour(0xff30333b));
 for(int i=0;i<5;++i){float y=plot.getY()+i*plot.getHeight()/4;g.drawHorizontalLine(int(y),plot.getX(),plot.getRight());}
 for(int i=0;i<6;++i){float x=plot.getX()+i*plot.getWidth()/5;g.drawVerticalLine(int(x),plot.getY(),plot.getBottom());}
 if(result.progress>0 || result.valid){juce::Path path;
  for(size_t i=0;i<128;++i){float x=plot.getX()+float(i)/127*plot.getWidth();float y=plot.getBottom()-juce::jlimit(0.0f,1.0f,(result.spectrum[i]+90)/90)*plot.getHeight();if(i==0)path.startNewSubPath(x,y);else path.lineTo(x,y);}
  g.setColour(gold.withAlpha(.9f));g.strokePath(path,juce::PathStrokeType(2));
 }else{g.setColour(muted);g.drawText("Analiza una frase para ver su balance tonal",plot,juce::Justification::centred);}
 g.setColour(muted);g.setFont(juce::FontOptions(11));for(float hz:{30.0f,250.0f,2000.0f,6000.0f,18000.0f}){
  float x=plot.getX()+std::log(hz/30.0f)/std::log(600.0f)*plot.getWidth();
  juce::String label=hz>=1000?juce::String(hz/1000,0)+" kHz":juce::String(hz,0)+" Hz";
  g.drawText(label,int(x)-24,graph.getBottom()-23,48,18,juce::Justification::centred);
 }
 g.drawText("0 dBFS",graph.getX()+7,graph.getY()+4,70,18,juce::Justification::centredLeft);
 g.drawText("-90",graph.getRight()-38,graph.getBottom()-45,34,18,juce::Justification::centredRight);
 g.setColour(gold.withAlpha(.6f));g.fillRect(graph.getX(),graph.getBottom()+10,int(graph.getWidth()*result.progress),2);
 g.setColour(muted);g.setFont(juce::FontOptions(12));g.drawText("COLOR SELECTIVO  /  SOBREMUESTREO 2x  /  MEZCLA PARALELA",graph.getX(),graph.getBottom()+24,graph.getWidth(),22,juce::Justification::centredLeft);
 auto db=[](float x){return juce::String(juce::Decibels::gainToDecibels(x,-80.0f),1);};
 g.setColour(proc.outputPeak.load()>1?juce::Colours::salmon:muted);
 g.drawText("IN "+db(proc.inputPeak)+" dBFS   OUT "+db(proc.outputPeak)+" dBFS   MATCH "+juce::String(proc.compensation.load(),1)+" dB",250,getHeight()-60,graph.getWidth()-220,26,juce::Justification::centredRight);
}

#include <JuceHeader.h>
#include <array>
#include <cmath>
using namespace juce;
using AP=AudioProcessorValueTreeState;
static String id(int c,int b,int p){return "c"+String(c)+"b"+String(b)+"p"+String(p);}
static const double gains[]={-9,-8,-7,-6,-5,-4,-3,-2.5,-2,-1.5,-1,-.5,0,.5,1,1.5,2,2.5,3,4,5,6,7,8,9};
static const double freqs[3][24]={{11,13,15,17,20,24,28,33,39,46,54,63,75,88,100,120,140,170,200,240,280,330,420,570},{125,150,180,210,250,300,350,420,500,600,700,850,1000,1200,1400,1700,2000,2400,2800,3400,4000,4800,5700,6800},{3400,3700,4000,4400,4800,5200,5700,6200,6800,7400,8000,8800,9600,10500,11500,12500,13600,14800,16000,17000,18500,20000,22500,25000}};
static const double q1[]={.67,1.03,1.77,2.30,4.38},q6[]={.53,.82,1.39,1.83,3.05};
class Engine:public AudioProcessor{
public:
 AP p{*this,nullptr,"MES432",layout()};
 static AP::ParameterLayout layout(){AP::ParameterLayout l;l.add(std::make_unique<AudioParameterChoice>(ParameterID{"color",1},"Output Color",StringArray{"Digital","Tube PP","Tube SE","Transistor PP","IC"},0));for(int c=0;c<2;++c){String ch=c?"Right ":"Left ";l.add(std::make_unique<AudioParameterBool>(ParameterID{"in"+String(c),1},ch+"EQ in",true));for(int b=0;b<5;++b){StringArray g,f;for(auto v:gains)g.add(String(v,1));if(b==0)f={"50","100"};else if(b==4)f={"10k"};else for(auto v:freqs[b-1])f.add(String(v,0));l.add(std::make_unique<AudioParameterChoice>(ParameterID{id(c,b,0),1},ch+"Gain "+String(b),g,12));if(b!=4)l.add(std::make_unique<AudioParameterChoice>(ParameterID{id(c,b,1),1},ch+"Frequency "+String(b),f,b==0?0:12));if(b>0&&b<4)l.add(std::make_unique<AudioParameterChoice>(ParameterID{id(c,b,2),1},ch+"Shape "+String(b),StringArray{"5","6","9","11","15"},0));}}return l;}
 Engine():AudioProcessor(BusesProperties().withInput("Input",AudioChannelSet::stereo(),true).withOutput("Output",AudioChannelSet::stereo(),true)){}
 const String getName()const override{return "MES-432";}bool acceptsMidi()const override{return false;}bool producesMidi()const override{return false;}double getTailLengthSeconds()const override{return 0;}int getNumPrograms()override{return 1;}int getCurrentProgram()override{return 0;}void setCurrentProgram(int)override{}const String getProgramName(int)override{return{};}void changeProgramName(int,const String&)override{}
 bool isBusesLayoutSupported(const BusesLayout& l)const override{return l.getMainInputChannelSet()==l.getMainOutputChannelSet()&&(l.getMainOutputChannelSet()==AudioChannelSet::mono()||l.getMainOutputChannelSet()==AudioChannelSet::stereo());}
 struct Filter{double b0=0,b1=0,b2=0,a1=0,a2=0,z1=0,z2=0;void reset(){z1=z2=0;}double run(double x){double y=b0*x+z1;z1=b1*x-a1*y+z2;z2=b2*x-a2*y;return y;}};
 Filter states[2][5];double sr=44100; double dc[2]{}; int lastColor=0; std::unique_ptr<dsp::Oversampling<float>> os;
 int v(String s){return int(p.getRawParameterValue(s)->load());}
 void prepareToPlay(double s,int block)override{sr=s;for(auto& ch:states)for(auto& f:ch)f.reset();dc[0]=dc[1]=0;os=std::make_unique<dsp::Oversampling<float>>(size_t(getTotalNumOutputChannels()),2,dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,true,true);os->initProcessing(size_t(block));os->reset();lastColor=v("color");setLatencySamples(lastColor?int(std::round(os->getLatencyInSamples())):0);}void releaseResources()override{}
 void processBlock(AudioBuffer<float>& buf,MidiBuffer&)override{ScopedNoDenormals nd;for(int c=0;c<buf.getNumChannels();++c){if(!v("in"+String(c))){for(auto& f:states[c])f.reset();continue;}bool active[5]{};bool any=false;for(int b=0;b<5;++b){auto& f=states[c][b];double g=gains[v(id(c,b,0))];if(g==0){f.reset();continue;}any=active[b]=true;double A=std::pow(10.,g/20.)-1.;
 if(b==0||b==4){double fc=b==0?(g>0?104.922:107.916)*(v(id(c,b,1))?2:1):(g>0?4099.321:3805.346);
 double trim=b==0?(g>0?.96276648:1.02533953):(g>0?.926376903:.948855989);
 A=std::pow(10.,g*trim/20.)-1.;double k=std::tan(MathConstants<double>::pi*fc/sr);f.a1=(k-1)/(k+1);f.a2=f.b2=0;f.b0=A*(b==0?k:1)/(1+k);f.b1=b==0?f.b0:-f.b0;}
 else{double fc=freqs[b-1][v(id(c,b,1))];int shape=v(id(c,b,2));double slope=std::log(q6[shape]/q1[shape])/5;double q=q1[shape]*std::exp(slope*(std::abs(g)-1))*std::pow(10.,g/40.);
 // Positive-gain measured +10 dB anchors; interpolate from +6, do not treat negative curves as measured.
 double t=g>0?jlimit(0.,1.,(g-6.)/4.):0.;
 const double fScale[]={1.02980866,1.0184,1.03},gainScale[]={.968157889,.922202206,.851058138};
 const int anchorShape[]={4,2,0};const double anchorQ[]={4.31620816,1.8728784,.37862756};
 double qAt10=q1[shape]*std::exp(std::log(q6[shape]/q1[shape])/5*9)*std::pow(10.,10./40.);
 if(shape==anchorShape[b-1])q*=std::pow(anchorQ[b-1]/qAt10,t);
 fc*=std::pow(fScale[b-1],t);A=std::pow(10.,g*std::pow(gainScale[b-1],t)/20.)-1.;
 // Continuous prewarp extension avoids discontinuous response at 0.45 Fs.
 double u=fc/sr;double k=u<=.45?std::tan(MathConstants<double>::pi*u):std::tan(MathConstants<double>::pi*.45)+(u-.45)*MathConstants<double>::pi/std::pow(std::cos(MathConstants<double>::pi*.45),2);
 double norm=1/(1+k/q+k*k);f.b0=A*k/q*norm;f.b1=0;f.b2=-f.b0;f.a1=2*(k*k-1)*norm;f.a2=(1-k/q+k*k)*norm;}
 }if(!any)continue;auto* x=buf.getWritePointer(c);for(int i=0;i<buf.getNumSamples();++i){double dry=x[i],y=dry;for(int b=0;b<5;++b)if(active[b])y+=states[c][b].run(dry);x[i]=float(y);}}
 int mode=v("color");if(mode!=lastColor){if(os)os->reset();dc[0]=dc[1]=0;lastColor=mode;setLatencySamples(mode&&os?int(std::round(os->getLatencyInSamples())):0);}
 if(mode==0||!os)return;
 // Magnitudes allocated from published THD+N ceilings and qualitative spectra.
 // Calibration: 0 dBFS peak 1 kHz, lower levels naturally produce less distortion.
 // Smoothly bounded nonlinear correction; no hard clipping of the dry waveform.
 const double h2[]={0,.0014,.0086,.000018,0},h3[]={0,.00185,.0015,.000118,.0000008};
 dsp::AudioBlock<float> block(buf);auto up=os->processSamplesUp(block);
 double alpha=1-std::exp(-2*MathConstants<double>::pi*3/(sr*4));
 for(size_t c=0;c<up.getNumChannels();++c){auto* x=up.getChannelPointer(c);for(size_t i=0;i<up.getNumSamples();++i){double a=x[i],z=jlimit(-1.,1.,a);double even=2*h2[mode]*z*z;dc[c]+=alpha*(even-dc[c]);double odd=4*h3[mode]*(z*z*z-.75*z);x[i]=float(a+even-dc[c]+odd);}}
 os->processSamplesDown(block);
 }
 void getStateInformation(MemoryBlock& b)override{auto xml=p.copyState().createXml();copyXmlToBinary(*xml,b);}void setStateInformation(const void* d,int n)override{if(auto xml=getXmlFromBinary(d,n))if(xml->hasTagName(p.state.getType()))p.replaceState(ValueTree::fromXml(*xml));}
 bool hasEditor()const override{return true;}AudioProcessorEditor* createEditor()override;
};
class Metal:public LookAndFeel_V4{
public:
 Metal(){setColour(ComboBox::backgroundColourId,Colour(0xff202020));setColour(ComboBox::textColourId,Colours::ivory);setColour(ComboBox::outlineColourId,Colour(0xff777777));}
 void drawRotarySlider(Graphics& g,int x,int y,int w,int h,float pos,float start,float end,Slider& slider)override{
 float cx=x+w*.5f,cy=y+h*.5f,r=jmin(w,h)*.265f;float angle=start+pos*(end-start);Rectangle<float> b(cx-r,cy-r,2*r,2*r);
 int count=int(slider.getProperties().getWithDefault("ticks",24));StringArray marks;marks.addTokens(slider.getProperties().getWithDefault("marks","").toString(),"|","");
 for(int j=0;j<count;++j){float a=start+(end-start)*j/float(jmax(1,count-1));float sx=std::sin(a),sy=-std::cos(a);g.setColour(Colour(0xffc6c4b9));g.drawLine(cx+sx*(r+3),cy+sy*(r+3),cx+sx*(r+8),cy+sy*(r+8),.8f);
 if(j<marks.size()){float rr=r+15;g.setFont(count>10?8.f:10.f);g.drawText(marks[j],int(cx+sx*rr-19),int(cy+sy*rr-6),38,12,Justification::centred);}}
 g.setColour(Colours::black.withAlpha(.85f));g.fillEllipse(b.translated(2,4).expanded(3));g.setColour(Colour(0xffa52c20));g.fillEllipse(b.translated(0,3).expanded(1));
 Path wedge;wedge.startNewSubPath(cx+std::sin(angle)*(r+7),cy-std::cos(angle)*(r+7));wedge.lineTo(cx+std::sin(angle-.14f)*r,cy-std::cos(angle-.14f)*r);wedge.lineTo(cx+std::sin(angle+.14f)*r,cy-std::cos(angle+.14f)*r);wedge.closeSubPath();g.fillPath(wedge);
 Path fluted;for(int j=0;j<144;++j){float a=j*MathConstants<float>::twoPi/144;float rr=r*(.96f+.04f*std::cos(24*a));float xx=cx+rr*std::sin(a),yy=cy-rr*std::cos(a);if(j==0)fluted.startNewSubPath(xx,yy);else fluted.lineTo(xx,yy);}fluted.closeSubPath();
 ColourGradient silver(Colour(0xfff2eee1),cx-r,cy-r,Colour(0xff55534e),cx+r,cy+r,false);silver.addColour(.35,Colour(0xffaaa69a));silver.addColour(.58,Colour(0xffe3dfd0));g.setGradientFill(silver);g.fillPath(fluted);
 for(int j=0;j<24;++j){float a=j*MathConstants<float>::twoPi/24;g.setColour(Colour(0xff4f4b42).withAlpha(.7f));g.drawLine(cx+std::sin(a)*(r-1),cy-std::cos(a)*(r-1),cx+std::sin(a)*(r-5),cy-std::cos(a)*(r-5),1.5f);}
 auto cap=b.reduced(4);g.setColour(Colour(0xffa8a59b));g.fillEllipse(cap.expanded(.6f));g.setGradientFill(ColourGradient(Colour(0xff222325),cx-r,cy-r,Colour(0xff030405),cx+r,cy+r,false));g.fillEllipse(cap);
 g.setColour(Colour(0xfff5f1df));g.drawLine(cx+std::sin(angle)*r*.10f,cy-std::cos(angle)*r*.10f,cx+std::sin(angle)*r*.81f,cy-std::cos(angle)*r*.81f,2.1f);
 }
 void drawToggleButton(Graphics& g,ToggleButton& t,bool,bool)override{auto r=t.getLocalBounds().toFloat();float x=r.getX()+r.getWidth()*.5f,y=r.getY()+12;bool on=t.getToggleState();
 g.setColour(Colour(0xff111211));g.fillEllipse(x-8,y-8,16,16);g.setGradientFill(ColourGradient(Colour(0xffdedaca),x-7,y-7,Colour(0xff55534d),x+7,y+7,false));g.fillEllipse(x-6,y-6,12,12);g.setColour(Colour(0xff25251f));g.fillEllipse(x-4,y-4,8,8);float yy=y+(on?-7:7);g.setColour(Colour(0xff99998e));g.drawLine(x,y,x,yy,5);g.setColour(Colour(0xffe8e5d6));g.fillEllipse(x-3,yy-3,6,6);g.setColour(Colours::ivory);g.setFont(9.f);g.drawText(t.getButtonText(),0,22,t.getWidth(),13,Justification::centred);}
};
class Editor:public AudioProcessorEditor,private Timer{
 Engine& e;Metal look;ComboBox color;std::unique_ptr<AP::ComboBoxAttachment> ca;
 std::vector<std::unique_ptr<Slider>> knobs;
 std::vector<std::unique_ptr<ToggleButton>> switches;
 std::vector<std::unique_ptr<TextButton>> eqButtons;
 std::vector<std::unique_ptr<AP::SliderAttachment>> sa;
 std::vector<std::unique_ptr<AP::ButtonAttachment>> ba;
 int channels=0;int panelWidth=0;static constexpr int faceHeight=350;static constexpr int height=390;
 void knob(String name,int cx,int cy,int size,int ticks,String marks){auto z=std::make_unique<Slider>();z->setSliderStyle(Slider::RotaryHorizontalVerticalDrag);z->setRotaryParameters(3.6651914f,8.9011792f,true);z->setTextBoxStyle(Slider::NoTextBox,false,0,0);z->getProperties().set("ticks",ticks);z->getProperties().set("marks",marks);z->setBounds(cx-size/2,cy-size/2,size,size);z->setTooltip(e.p.getParameter(name)->getName(100));addAndMakeVisible(*z);sa.push_back(std::make_unique<AP::SliderAttachment>(e.p,name,*z));knobs.push_back(std::move(z));}
 String gainMarks(){StringArray a;for(int j=0;j<25;++j)a.add(j%2==0||j==12?String(gains[j],gains[j]==int(gains[j])?0:1):" ");return a.joinIntoString("|");}
 String freqMarks(int b){StringArray a;for(int j=0;j<24;++j){double f=freqs[b-1][j];a.add(j%2==0||j==23?(f>=1000?String(f/1000, f==int(f/1000)*1000?0:1)+"k":String(f,0)):" ");}return a.joinIntoString("|");}
 void makeSwitch(int c,int x,int y){auto t=std::make_unique<TextButton>(c?"EQ RIGHT IN":"EQ LEFT IN");t->setClickingTogglesState(true);t->setColour(TextButton::buttonColourId,Colour(0xff292929));t->setColour(TextButton::buttonOnColourId,Colour(0xff31543c));t->setColour(TextButton::textColourOffId,Colour(0xffb0b0ac));t->setColour(TextButton::textColourOnId,Colour(0xffe2f0df));t->setBounds(x-63,y,126,26);addAndMakeVisible(*t);ba.push_back(std::make_unique<AP::ButtonAttachment>(e.p,"in"+String(c),*t));eqButtons.push_back(std::move(t));}
 int shelfX(int c)const{return c?679:105;}int bellX(int c,int b)const{return (c?773:199)+(b-1)*105;}
 void rebuild(){int n=e.getTotalNumInputChannels()==1?1:2;if(n==channels)return;channels=n;sa.clear();ba.clear();knobs.clear();switches.clear();eqButtons.clear();panelWidth=n==1?580:1070;setSize(panelWidth,height);
 color.setBounds(panelWidth-162,4,149,24);
 for(int c=0;c<n;++c){int sx=shelfX(c);knob(id(c,4,0),sx,157,104,25,gainMarks());knob(id(c,0,0),sx,275,104,25,gainMarks());
 auto t=std::make_unique<ToggleButton>("50 / 100 Hz");t->setBounds(sx-40,208,80,36);auto* ptr=t.get();auto param=id(c,0,1);t->setToggleState(e.v(param)!=0,dontSendNotification);t->onClick=[this,ptr,param]{auto* p=e.p.getParameter(param);p->beginChangeGesture();p->setValueNotifyingHost(ptr->getToggleState()?1.f:0.f);p->endChangeGesture();};t->getProperties().set("shelfChannel",c);addAndMakeVisible(*t);switches.push_back(std::move(t));
 for(int b=1;b<=3;++b){int x=bellX(c,b);knob(id(c,b,2),x,81,92,5,"5|6|9|11|15");knob(id(c,b,1),x,179,112,24,freqMarks(b));knob(id(c,b,0),x,278,104,25,gainMarks());}
 makeSwitch(c,n==1?panelWidth/2:(c?panelWidth/2+71:panelWidth/2-71),357);
 }repaint();}
 void timerCallback()override{rebuild();for(auto& t:switches)if(t->getProperties().contains("shelfChannel"))t->setToggleState(e.v(id(int(t->getProperties()["shelfChannel"]),0,1))!=0,dontSendNotification);repaint();}
public:
 Editor(Engine& a):AudioProcessorEditor(a),e(a){setLookAndFeel(&look);setOpaque(true);color.addItemList(StringArray{"Digital","Tube PP","Tube SE","Transistor PP","IC"},1);addAndMakeVisible(color);ca=std::make_unique<AP::ComboBoxAttachment>(e.p,"color",color);rebuild();startTimerHz(5);}
 ~Editor()override{stopTimer();setLookAndFeel(nullptr);}
 void paint(Graphics& g)override{
 g.fillAll(Colour(0xff151616));g.setGradientFill(ColourGradient(Colour(0xff383939),0,30,Colour(0xff242525),0,350,false));g.fillRect(0,30,getWidth(),faceHeight-30);g.setColour(Colour(0xff080909));g.fillRect(0,faceHeight,getWidth(),height-faceHeight);g.setColour(Colour(0xff555650));g.drawHorizontalLine(faceHeight,0,float(getWidth()));
 g.setColour(Colour(0xff0c0d0d));g.fillRect(0,0,getWidth(),30);g.setColour(Colour(0xffddd9ca));g.setFont(12.f);g.drawText("OUTPUT COLOR",getWidth()-260,4,94,24,Justification::centredRight);
 for(int x:{8,getWidth()-27})for(int y:{64,300}){g.setColour(Colour(0xff080909));g.fillRoundedRectangle(float(x),float(y),19,9,4);g.setColour(Colour(0xff8b8980));g.drawRoundedRectangle(float(x),float(y),19,9,4,.8f);}
 auto line=[&](int x){g.setColour(Colour(0xffa4a296));g.drawVerticalLine(x,46,321);g.setColour(Colour(0xff77786f));g.drawVerticalLine(x+2,46,321);};
 auto txt=[&](String t,int x,int y,int w,int h,float size){g.setColour(Colour(0xffe0ddd0));g.setFont(size);g.drawText(t,x,y,w,h,Justification::centred);};
 for(int c=0;c<channels;++c){int sx=shelfX(c);line(sx-52);line(sx+52);txt("SHELVING",sx-50,98,100,14,10);txt("HI",sx-54,112,25,12,9);txt("LO",sx-54,238,25,12,9);
 for(int b=1;b<=3;++b){int x=bellX(c,b);if(b>1)line(x-52);txt(b==1?"LO":b==2?"MID":"HI",x-45,125,90,13,11);txt("Hz",x-42,222,84,12,9);}
 line(bellX(c,3)+52);int loc=channels==1?520:(c?555:495);txt(c?"RIGHT":"LEFT",loc-28,84,56,16,10);
 }
 txt("PARAMETRIC MASTERING EQUALIZER",48,325,channels==1?330:430,18,12);txt("MES-432",getWidth()-134,325,100,18,13);
 }
};
AudioProcessorEditor* Engine::createEditor(){return new Editor(*this);}
AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new Engine();}

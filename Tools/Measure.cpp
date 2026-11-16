#include <juce_audio_utils/juce_audio_utils.h>
#include <fstream>
#include <iostream>
using namespace juce;
int main(int argc,char** argv){
 ScopedJuceInitialiser_GUI init;if(argc!=3)return 1;
 AudioPluginFormatManager fm;fm.addDefaultFormats();OwnedArray<PluginDescription> types;
 for(auto* f:fm.getFormats())if(f->getName()=="VST3")f->findAllTypesForFile(types,String::fromUTF8(argv[1]));
 if(types.isEmpty())return 2;String error;auto p=fm.createPluginInstance(*types[0],48000,512,error);if(!p){std::cerr<<error;return 3;}
 auto params=p->getParameters();auto set=[&](String name,int index){for(auto* v:params)if(v->getName(256)==name){v->setValueNotifyingHost(float(index)/float(v->getNumSteps()-1));return;}throw std::runtime_error(name.toStdString());};
 std::ofstream out(argv[2],std::ios::binary);if(!out)return 4;
 constexpr int N=32768;AudioBuffer<float> block(2,512);MidiBuffer midi;
 for(int b=0;b<5;++b)for(int f=0;f<(b==0?2:b==4?1:24);++f)for(int q=0;q<((b>0&&b<4)?5:1);++q)for(int g=0;g<25;++g){
 if(g==12)continue;
 for(int j=0;j<5;++j)set("Left Gain "+String(j),12);
 set("Left Gain "+String(b),g);if(b!=4)set("Left Frequency "+String(b),f);if(b>0&&b<4)set("Left Shape "+String(b),q);
 p->releaseResources();p->prepareToPlay(48000,512);
 int header[]={b,f,q,g};out.write(reinterpret_cast<char*>(header),sizeof(header));
 for(int i=0;i<N;i+=512){block.clear();if(i==0)block.setSample(0,0,1.f);p->processBlock(block,midi);out.write(reinterpret_cast<const char*>(block.getReadPointer(0)),512*sizeof(float));}
 }
 return out?0:5;
}

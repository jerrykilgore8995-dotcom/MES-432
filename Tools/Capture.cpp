#include <juce_audio_utils/juce_audio_utils.h>
#include <windows.h>
#include <iostream>
using namespace juce;
int main(int argc,char** argv){
 ScopedJuceInitialiser_GUI init;if(argc!=4)return 1;
 AudioPluginFormatManager fm;fm.addDefaultFormats();OwnedArray<PluginDescription> types;
 for(auto* f:fm.getFormats())if(f->getName()=="VST3")f->findAllTypesForFile(types,String::fromUTF8(argv[1]));
 if(types.isEmpty())return 2;String error;auto p=fm.createPluginInstance(*types[0],48000,512,error);if(!p){std::cerr<<error;return 3;}
 bool mono=String(argv[2])=="mono";AudioProcessor::BusesLayout layout;layout.inputBuses.add(mono?AudioChannelSet::mono():AudioChannelSet::stereo());layout.outputBuses.add(mono?AudioChannelSet::mono():AudioChannelSet::stereo());
 if(!p->setBusesLayout(layout)||p->getTotalNumInputChannels()!=(mono?1:2)){std::cerr<<"Bus negotiation failed";return 4;}
 p->prepareToPlay(48000,512);
 std::unique_ptr<AudioProcessorEditor> ed(p->createEditorIfNeeded());if(!ed)return 5;
 ed->setTopLeftPosition(10,10);ed->addToDesktop(0);ed->setVisible(true);ed->toFront(true);
 auto until=GetTickCount64()+2000;while(GetTickCount64()<until){MSG msg;while(PeekMessage(&msg,nullptr,0,0,PM_REMOVE)){TranslateMessage(&msg);DispatchMessage(&msg);}Sleep(10);}
 HWND hwnd=static_cast<HWND>(ed->getWindowHandle());if(!hwnd)return 6;
 RECT rc{};GetClientRect(hwnd,&rc);int w=rc.right,h=rc.bottom;std::cout<<"Captured editor "<<w<<" x "<<h<<" channels "<<p->getTotalNumInputChannels()<<std::endl;
 if(w<100||h<100|| (mono&&w>=h*2.3)||(!mono&&w<=h*2.3))return 7;
 HDC dc=GetDC(hwnd),mem=CreateCompatibleDC(dc);BITMAPINFO bmi{};bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);bmi.bmiHeader.biWidth=w;bmi.bmiHeader.biHeight=-h;bmi.bmiHeader.biPlanes=1;bmi.bmiHeader.biBitCount=32;bmi.bmiHeader.biCompression=BI_RGB;
 void* pixels=nullptr;HBITMAP bmp=CreateDIBSection(dc,&bmi,DIB_RGB_COLORS,&pixels,nullptr,0);if(!bmp||!pixels)return 8;auto old=SelectObject(mem,bmp);
 auto capture=[&]{Image im(Image::RGB,w,h,true);auto* d=static_cast<unsigned char*>(pixels);for(int y=0;y<h;++y)for(int x=0;x<w;++x){auto* b=d+4*(y*w+x);im.setPixelAt(x,y,Colour(b[2],b[1],b[0]));}return im;};
 auto good=[](const Image& im){int bright=0;for(int y=0;y<im.getHeight();y+=3)for(int x=0;x<im.getWidth();x+=3)if(im.getPixelAt(x,y).getBrightness()>.55f)++bright;return bright>200;};
 PrintWindow(hwnd,mem,3);auto im=capture();if(!good(im)){BitBlt(mem,0,0,w,h,dc,0,0,SRCCOPY);im=capture();}
 SelectObject(mem,old);DeleteObject(bmp);DeleteDC(mem);ReleaseDC(hwnd,dc);
 if(!good(im)){std::cerr<<"Blank screenshot";return 9;}
 File file=File::getCurrentWorkingDirectory().getChildFile(String(argv[3]));file.deleteFile();FileOutputStream stream(file);PNGImageFormat png;if(!stream.openedOk()||!png.writeImageToStream(im,stream))return 10;stream.flush();ed->removeFromDesktop();return 0;
}

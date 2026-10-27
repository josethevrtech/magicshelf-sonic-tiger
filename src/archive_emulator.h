#pragma once
#include "render.h"
#include "archive_shared.h"
#include <QCoreApplication>
#include <QProcess>
#include <QProcessEnvironment>
#include <QDir>
#include <QFile>
#include <QSaveFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QImage>
#include <atomic>
#include <thread>
#include <mutex>
#include <memory>
#include <filesystem>
struct SonicFrame{bool menu=false,mute=false,loaded=false;int selection=0,volume=28;unsigned inputs=0;std::string error;};
class Emulator {
 std::string data;QProcess player;std::unique_ptr<ArchiveMapping> mapping;
 std::shared_ptr<const SonicFrame> latest=std::make_shared<SonicFrame>();SonicFrame state;
 std::atomic<unsigned> buttons{0},modelButtons{0},modelTaps{0};std::atomic<bool> stopping{false},saveRequested{false};
 bool noAudio=qEnvironmentVariableIsSet("SONIC_NO_AUDIO");
 std::thread worker;std::mutex mutex;std::vector<QString> commands;
 std::vector<unsigned char> pixels=std::vector<unsigned char>(ArchiveWidth*ArchiveHeight*4);uint32_t lastFrame=0;
 static unsigned mask(const QString& b){if(b=="left")return 1;if(b=="top")return 2;if(b=="right")return 4;if(b=="bottom")return 8;if(b=="enter")return 16;if(b=="back")return 32;if(b=="sound")return 64;if(b=="state")return 128;if(b=="power")return 256;if(b=="reset")return 512;return 0;}
 void persist(){QSaveFile f(QString::fromStdString(data+"/online-settings.json"));if(f.open(QIODevice::WriteOnly)){f.write(QJsonDocument(QJsonObject{{"mute",state.mute},{"volume",state.volume}}).toJson());f.commit();}}
 void publish(){std::atomic_store(&latest,std::make_shared<const SonicFrame>(state));}
 void run(){
  unsigned pulses=0,previousInput=0,latchedModel=0;int pulseTicks=0,modelTicks=0;auto next=std::chrono::steady_clock::now(),saved=next;
  while(!stopping){
   auto* shared=mapping->shared;std::vector<QString> pending;{std::lock_guard<std::mutex> lock(mutex);pending.swap(commands);}
   for(auto& b:pending){
    if(b=="menu"){state.menu=!state.menu;buttons=0;modelButtons=0;modelTaps=0;latchedModel=0;modelTicks=0;}
    else if(state.menu){
     if(b=="top")state.selection=(state.selection+3)%4;if(b=="bottom")state.selection=(state.selection+1)%4;
     if(b=="back")state.menu=false;
     if(b=="left"||b=="right")state.volume=std::clamp(state.volume+(b=="right"?5:-5),0,100);
     if(b=="enter"){
      if(state.selection==0)state.menu=false;else if(state.selection==1)state.mute=!state.mute;
      else if(state.selection==2)state.volume=(state.volume+5)%105;
      else{shared->reload.fetch_add(1);state.menu=false;}
     }
    }else if(!state.loaded&&b=="enter"){shared->reload.fetch_add(1);}
    else {pulses|=mask(b);pulseTicks=10;}
   }
   auto status=shared->status.load();state.loaded=status==1;state.error=status==2?"Cannot reach Archive.org. A: retry":"Connecting to Internet Archive...";
   unsigned tap=modelTaps.exchange(0);if(tap){latchedModel=tap;modelTicks=12;}
   unsigned input=state.menu?0:(buttons.load()|modelButtons.load()|pulses|latchedModel);
   if(modelTicks>0&&!--modelTicks)latchedModel=0;
   if(status==2&&(input&16)&&!(previousInput&16))shared->reload.fetch_add(1);
   previousInput=input;
   shared->keys=input;state.inputs=input;if(pulseTicks>0&&!--pulseTicks)pulses=0;
   shared->gain=noAudio||state.mute||state.menu?0:state.volume;
   ticks=shared->frames.load();audioConnected=status==1;audioPeak=shared->audible.load()?1:0;
   deliveredKeys=shared->deliveredKeys.load();deliveredEvents=shared->deliveredEvents.load();
   publish();auto now=std::chrono::steady_clock::now();if(saveRequested.exchange(false)||now-saved>std::chrono::seconds(2)){persist();saved=now;}
   next+=std::chrono::milliseconds(10);if(now-next>std::chrono::milliseconds(100))next=now;std::this_thread::sleep_until(next);
  }
  mapping->shared->keys=0;persist();
 }
public:
 std::atomic<unsigned long> ticks{0},audioSamples{0};std::atomic<bool> audioConnected{false};std::atomic<int> audioPeak{0};
 std::atomic<unsigned> deliveredKeys{0},deliveredEvents{0};
 Emulator(QString package,QString directory):data(directory.toStdString()){
  std::filesystem::create_directories(data);QFile settings(directory+"/online-settings.json");if(settings.open(QIODevice::ReadOnly)){auto o=QJsonDocument::fromJson(settings.readAll()).object();state.mute=o["mute"].toBool();state.volume=std::clamp(o["volume"].toInt(28),0,100);}
  mapping=std::make_unique<ArchiveMapping>(data+"/archive-frame.bin",true);mapping->shared->gain=noAudio||state.mute?0:state.volume;
  auto env=QProcessEnvironment::systemEnvironment();env.insert("QT_QPA_PLATFORM","offscreen");
  // Keep Chromium sandboxing enabled. Only rendering is forced to software;
  // the player's process and frame copies never run on the tracking thread.
  env.insert("QTWEBENGINE_CHROMIUM_FLAGS","--disable-gpu --autoplay-policy=no-user-gesture-required");
  player.setProcessEnvironment(env);player.setProcessChannelMode(QProcess::ForwardedChannels);
  player.start(package+"/bin/archive-player",{QString::fromStdString(data+"/archive-frame.bin")});
  if(!player.waitForStarted(4000))throw std::runtime_error("Cannot start the online player");
  publish();worker=std::thread([this]{run();});
 }
 ~Emulator(){stopping=true;if(worker.joinable())worker.join();mapping->shared->quit=1;if(!player.waitForFinished(3000)){player.terminate();if(!player.waitForFinished(2000)){player.kill();player.waitForFinished(1000);}}}
 bool loaded()const{return snapshot()->loaded;}
 std::shared_ptr<const SonicFrame> snapshot()const{return std::atomic_load(&latest);}
 void save(){saveRequested=true;}
 void setButtons(unsigned b){buttons=b;}
 void press(const QString& b){std::lock_guard<std::mutex> lock(mutex);if(commands.size()<64)commands.push_back(b);}
 void key(const QString& b,bool down){if(b=="menu"||snapshot()->menu){if(down)press(b);return;}unsigned bit=mask(b);if(down)buttons.fetch_or(bit);else buttons.fetch_and(~bit);}
 void modelDown(const QString& b){if(snapshot()->menu){modelButtons=0;press(b);}else{unsigned bit=mask(b);modelButtons=bit;modelTaps.fetch_or(bit);}}
 void modelUp(){modelButtons=0;}
 void drawLCD(Canvas& canvas){
  auto frame=snapshot();
  if(frame->menu){canvas.fill(.06,.12,.17);canvas.text(30,55,"SONIC / INTERNET ARCHIVE",30);std::string items[]={"Resume game",std::string("Sound: ")+(frame->mute?"off":"on"),"Volume: "+std::to_string(frame->volume)+"%","Reload online player"};for(int i=0;i<4;i++){if(i==frame->selection)canvas.rect(20,85+i*68,718,56,.12,.28,.37);canvas.text(40,123+i*68,items[i],27);}canvas.text(30,420,"Arrows choose / A select / B return",22);canvas.text(30,465,"Archive.org player / internet required",20);canvas.text(30,505,"Model: Mattyew / CC BY 4.0 (Sonic 2 casing)",18);return;}
  if(!frame->loaded){canvas.fill(.06,.12,.17);canvas.text(30,80,"SONIC THE HEDGEHOG",34);canvas.text(30,155,frame->error,25);canvas.text(30,230,"Online player - no ROM import",25);canvas.text(30,300,"X or ON/START: start / A: spin",24);canvas.text(30,375,"Y: settings and retry",24);canvas.text(30,485,"Hosted by Internet Archive / archive.org",20);return;}
  auto* shared=mapping->shared;auto seq=shared->frameSequence.load(std::memory_order_acquire);
  if(seq&&seq!=lastFrame&&!(seq&1)){
   std::vector<unsigned char> candidate(pixels.size());memcpy(candidate.data(),shared->pixels,candidate.size());
   if(shared->frameSequence.load(std::memory_order_acquire)==seq){pixels.swap(candidate);lastFrame=seq;}
  }
  auto surface=cairo_image_surface_create_for_data(pixels.data(),CAIRO_FORMAT_ARGB32,ArchiveWidth,ArchiveHeight,ArchiveWidth*4);
  cairo_set_source_surface(canvas.c,surface,0,0);cairo_paint(canvas.c);cairo_surface_destroy(surface);
 }
};

#include "emulator.h"
#include <QTemporaryDir>
#include <cassert>
#include <set>
int main(int argc,char**argv){
 QCoreApplication app(argc,argv);
 // Exercise actual Sonic electrical rows: Start is fixed, S1 is audio only.
 sonic_buttons=128;assert(gw_readK(0)==8);assert(gw_readK(255)==8);
 sonic_buttons=2;assert(gw_readK(2)==4);assert(gw_readK(1)==0);
 sonic_buttons=1|4;assert(gw_readK(4)==6);
 sonic_buttons=8;assert(gw_readK(8)==2);
 sonic_buttons=16;assert(gw_readK(16)==1);
 sonic_buttons=32;assert(gw_readK(32)==1);
 sonic_buttons=64;assert(gw_readBA()==0&&gw_readB()==1);
 sonic_buttons=256;assert(gw_readB()==0&&gw_readBA()==1);
 // All four DAC levels, including S1 weight and exactly one sample per call.
 sonic_audio_count=0;for(int s=0;s<2;s++)for(int r=0;r<2;r++){m_s_out=s;gw_writeR(r);}
 assert(sonic_audio_count==4);for(int i=0;i<4;i++)assert(sonic_audio[i]==i*6000-9000);
 QTemporaryDir dir;
 if(!QFile::exists(QString::fromLocal8Bit(qgetenv("SONIC_CONTENT_DIR"))+"/n71.program")){
  Emulator missing(".",dir.path());assert(!missing.loaded());
  std::cout<<"Input matrix, DAC and missing-content checks passed; private ROM tests skipped\n";return 0;
 }
 {
  Emulator game(".",dir.path());assert(game.loaded());game.press("state");
  std::set<std::array<bool,256>> frames;auto start=std::chrono::steady_clock::now();
  while(std::chrono::steady_clock::now()-start<std::chrono::seconds(9)){
   auto f=game.snapshot();frames.insert(f->segments);std::this_thread::sleep_for(std::chrono::milliseconds(20));
   if(game.ticks>180&&game.ticks<185)game.press("state");
   if(game.ticks>500&&game.ticks<900)game.setButtons(4|16);
   if(game.ticks>=900)game.setButtons(0);
  }
  assert(game.ticks>900);assert(frames.size()>10);
  game.press("menu");std::this_thread::sleep_for(std::chrono::milliseconds(30));assert(game.snapshot()->menu);
  game.press("bottom");game.press("enter");std::this_thread::sleep_for(std::chrono::milliseconds(30));assert(game.snapshot()->mute);
  game.press("back");game.setButtons(1);std::this_thread::sleep_for(std::chrono::milliseconds(30));game.setButtons(0);game.save();
  Canvas lcd(758,540);game.drawLCD(lcd);lcd.png(dir.path().toStdString()+"/lcd.png");
  std::cout<<"ROM executed: "<<game.ticks<<" ticks, "<<frames.size()<<" distinct LCD states\n";
 }
 QFile saved(dir.path()+"/state.bin");assert(saved.open(QIODevice::ReadOnly));auto bytes=saved.readAll();assert(bytes.size()==32+sizeof(gw_state_t));
 {Emulator restored(".",dir.path());assert(restored.loaded());assert(restored.snapshot()->mute);}
 // Exact deterministic state replay through the CPU and original ROM, without worker.
 gw_state_t checkpoint,future,replay;gw_state_save(reinterpret_cast<unsigned char*>(&checkpoint));
 auto advance=[](){sonic_buttons=4;for(int i=0;i<256;i++){sonic_audio_count=0;gw_system_run(256);}};
 advance();gw_state_save(reinterpret_cast<unsigned char*>(&future));assert(gw_state_load(reinterpret_cast<unsigned char*>(&checkpoint)));
 advance();gw_state_save(reinterpret_cast<unsigned char*>(&replay));assert(memcmp(&future,&replay,sizeof(future))==0);
 // Missing content remains usable as a local import screen.
 qputenv("SONIC_CONTENT_DIR","/nonexistent-sonic-content");{Emulator missing(".",dir.path());assert(!missing.loaded());}
 std::cout<<"Input matrix, DAC, live ROM, settings, save replay, missing-content checks passed\n";
}

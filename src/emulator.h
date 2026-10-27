#pragma once
#include "render.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QSaveFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QCryptographicHash>
#include <QImage>
#include <pulse/simple.h>
#include <atomic>
#include <thread>
#include <mutex>
#include <memory>
#include <filesystem>
extern "C" {
#include "gw_system.h"
#include "gw_romloader.h"
#include "sm510.h"
extern unsigned sonic_buttons,sonic_audio_count;
extern short sonic_audio[2048];
}
struct SonicFrame {std::array<bool,256> segments{};bool menu=false,mute=false,loaded=false;int selection=0,volume=40;std::string error;};
class Emulator {
 struct Segment{int index,x,y;QImage image;};std::vector<Segment> art;
 std::string data,content;QByteArray program,melody;
 std::shared_ptr<const SonicFrame> latest=std::make_shared<SonicFrame>();
 std::atomic<unsigned> buttons{0},pulses{0};std::atomic<bool> stopping{false},saveRequested{false};
 std::thread worker;bool imported=false;SonicFrame state;pa_simple* audio=nullptr;
 unsigned heldPulse=0;int pulseTicks=0;float previousAudio=0,filteredAudio=0;
 std::mutex commandMutex;std::vector<QString> commands;
 static QByteArray read(const QString& p){QFile f(p);if(!f.open(QIODevice::ReadOnly))return {};return f.readAll();}
 static unsigned mask(const QString& b){if(b=="left")return 1;if(b=="top")return 2;if(b=="right")return 4;if(b=="bottom")return 8;if(b=="enter")return 16;if(b=="back")return 32;if(b=="sound")return 64;if(b=="state")return 128;if(b=="power")return 256;return 0;}
 void publish(){std::atomic_store(&latest,std::make_shared<const SonicFrame>(state));}
 void persist(){
  QJsonObject settings{{"mute",state.mute},{"volume",state.volume}};QSaveFile f(QString::fromStdString(data+"/settings.json"));if(f.open(QIODevice::WriteOnly)){f.write(QJsonDocument(settings).toJson());f.commit();}
  if(!imported)return;
  QByteArray blob(sizeof(gw_state_t),0);gw_state_save(reinterpret_cast<unsigned char*>(blob.data()));
  QSaveFile out(QString::fromStdString(data+"/state.bin"));if(out.open(QIODevice::WriteOnly)){out.write(QCryptographicHash::hash(blob,QCryptographicHash::Sha256));out.write(blob);out.commit();}
 }
 void restore(){
  auto bytes=read(QString::fromStdString(data+"/state.bin"));
  if(bytes.size()!=int(32+sizeof(gw_state_t)))return;auto blob=bytes.mid(32);
  if(bytes.left(32)!=QCryptographicHash::hash(blob,QCryptographicHash::Sha256))return;
  gw_state_t s;memcpy(&s,blob.data(),sizeof(s));
  if(s.m_stack_levels!=2||s.m_pc>=4096||s.m_prev_pc>=4096||s.m_melody_duty_index>3||s.m_acc>15||s.m_bl>15||s.m_bm>7||(s.m_clk_div!=2&&s.m_clk_div!=4)||s.m_icount>0||s.m_icount< -8)return;
  gw_state_load(reinterpret_cast<unsigned char*>(blob.data()));
 }
 void tick(){
  std::vector<QString> pending;{std::lock_guard<std::mutex> lock(commandMutex);pending.swap(commands);}
  for(auto& b:pending){
   if(b=="menu"){state.menu=!state.menu;buttons=0;}
   else if(state.menu){
    if(b=="top")state.selection=(state.selection+3)%4;
    if(b=="bottom")state.selection=(state.selection+1)%4;
    if(b=="back")state.menu=false;
    if(b=="left"||b=="right")state.volume=std::clamp(state.volume+(b=="right"?10:-10),0,100);
    if(b=="enter"){
     if(state.selection==0)state.menu=false;
     else if(state.selection==1)state.mute=!state.mute;
     else if(state.selection==2)state.volume=(state.volume+10)%110;
     else {state.menu=false;pulses.fetch_or(128);}
    }
   }else if(b=="reset"&&imported){gw_system_reset();}
   else pulses.fetch_or(mask(b));
  }
  if(imported){
   unsigned pulse=pulses.exchange(0);
   if(pulse){heldPulse|=pulse;pulseTicks=8;}
   sonic_buttons=state.menu?0:buttons.load()|heldPulse;
   if(pulseTicks>0&&!--pulseTicks)heldPulse=0;
   sonic_audio_count=0;gw_system_run(256);
   for(int x=0;x<4;x++)for(int y=0;y<16;y++)for(int z=0;z<4;z++){
    bool on=false;if(!m_bc&&m_bp){if(x!=2)on=(gw_ram_state[(x==0?0x60:x==1?0x70:0x50)+y]>>z)&1;
     else if(y<2){unsigned blink=(m_div&0x4000)?m_y:0;on=((y==0?m_l:m_x)&~blink)>>z&1;}}
    state.segments[x*64+y*4+z]=on;
   }
   if(audio&&sonic_audio_count){
    short samples[2048];for(unsigned i=0;i<sonic_audio_count;i++){
     float value=sonic_audio[i];filteredAudio=value-previousAudio+.995f*filteredAudio;previousAudio=value;
     samples[i]=state.mute||state.menu?0:std::clamp(filteredAudio*state.volume/100,-32767.f,32767.f);
    }
    int peak=0;for(unsigned i=0;i<sonic_audio_count;i++)peak=std::max(peak,std::abs(int(samples[i])));
    audioPeak=peak;audioSamples+=sonic_audio_count;
    int error;if(pa_simple_write(audio,samples,sonic_audio_count*sizeof(short),&error)<0){std::cerr<<"Audio write failed: "<<error<<"\n";pa_simple_free(audio);audio=nullptr;audioConnected=false;}
   }
  }
  ++ticks;publish();
 }
 void run(){
  if(imported&&!getenv("SONIC_NO_AUDIO")){pa_sample_spec spec{PA_SAMPLE_S16LE,32768,1};pa_buffer_attr attr{uint32_t(-1),4096,uint32_t(-1),1024,uint32_t(-1)};int error;audio=pa_simple_new(nullptr,"Sonic Tiger LCD",PA_STREAM_PLAYBACK,nullptr,"Handheld speaker",&spec,nullptr,&attr,&error);audioConnected=audio!=nullptr;if(!audio)std::cerr<<"Audio connection failed: "<<error<<"\n";}
  auto next=std::chrono::steady_clock::now(),saved=next;
  while(!stopping){tick();auto now=std::chrono::steady_clock::now();if(saveRequested.exchange(false)||now-saved>std::chrono::seconds(2)){persist();saved=now;}
   next+=std::chrono::nanoseconds(7812500);if(now-next>std::chrono::milliseconds(250))next=now;std::this_thread::sleep_until(next);}
  persist();if(audio)pa_simple_free(audio);
 }
public:
 std::atomic<unsigned long> ticks{0};
 std::atomic<bool> audioConnected{false};std::atomic<unsigned long> audioSamples{0};std::atomic<int> audioPeak{0};
 Emulator(QString package,QString directory):data(directory.toStdString()){
  (void)package;std::filesystem::create_directories(data);content=getenv("SONIC_CONTENT_DIR")?getenv("SONIC_CONTENT_DIR"):data+"/content";
  auto settings=QJsonDocument::fromJson(read(directory+"/settings.json")).object();state.mute=settings["mute"].toBool();state.volume=std::clamp(settings["volume"].toInt(40),0,100);
  try{
   program=read(QString::fromStdString(content+"/n71.program"));melody=read(QString::fromStdString(content+"/n71.melody"));
   auto hash=[](const QByteArray& b){return QCryptographicHash::hash(b,QCryptographicHash::Sha1).toHex();};
   if(hash(program)!="bf8d0ab88d153fabc688ffec19959209ca79c3db"||hash(melody)!="81cb75d73fab4479cd92fcb13d9cb03cec2afdd5")throw std::runtime_error("Import your tsonic.zip to play");
   auto segments=QJsonDocument::fromJson(read(QString::fromStdString(content+"/segments.json"))).array();
   for(auto v:segments){auto o=v.toObject();auto key=o["key"].toString();auto parts=key.split('.');if(parts.size()!=3)throw std::runtime_error("Invalid LCD segment map");int x=parts[0].toInt(),y=parts[1].toInt(),z=parts[2].toInt();if(x<0||x>3||y<0||y>15||z<0||z>3)throw std::runtime_error("Invalid segment address");auto box=o["box"].toArray();QImage im(QString::fromStdString(content+"/segments/")+key+".png");if(im.isNull()||box.size()!=4)throw std::runtime_error("LCD image missing");art.push_back({x*64+y*4+z,box[0].toInt(),box[1].toInt(),im.convertToFormat(QImage::Format_ARGB32_Premultiplied)});}
   if(art.size()!=132)throw std::runtime_error("LCD import incomplete");
   memset(&gw_head,0,sizeof(gw_head));memcpy(gw_head.cpu_name,"SM511",5);memcpy(gw_head.rom_signature,"tsonic01",8);gw_head.flags=FLAG_LCD_DEFLICKER_2;
   gw_program=reinterpret_cast<unsigned char*>(program.data());gw_melody=reinterpret_cast<unsigned char*>(melody.data());
   if(!gw_system_config())throw std::runtime_error("SM511 initialization failed");gw_system_start();gw_system_reset();restore();imported=state.loaded=true;
  }catch(const std::exception&e){state.error=e.what();}
  publish();worker=std::thread([this]{run();});
 }
 ~Emulator(){stopping=true;if(worker.joinable())worker.join();}
 bool loaded()const{return imported;}
 auto snapshot()const{return std::atomic_load(&latest);}
 void save(){saveRequested=true;}
 void setButtons(unsigned b){buttons=b;}
 void press(const QString& b){std::lock_guard<std::mutex> lock(commandMutex);if(commands.size()<64)commands.push_back(b);}
 void key(const QString& b,bool down){auto frame=snapshot();if(b=="menu"||frame->menu){if(down)press(b);return;}unsigned m=mask(b);if(down){if(b=="state"||b=="sound"||b=="power")press(b);else buttons.fetch_or(m);}else buttons.fetch_and(~m);}
 void drawLCD(Canvas& c){
  auto frame=snapshot();c.fill(.72,.76,.59);
  if(frame->menu){c.fill(.06,.12,.17);c.text(32,60,"SONIC TIGER LCD",32);std::string items[]={"Resume game",std::string("Sound: ")+(frame->mute?"off":"on"),"Volume: "+std::to_string(frame->volume)+"%","Power on / Start"};for(int i=0;i<4;i++){if(i==frame->selection)c.rect(20,92+i*68,718,56,.12,.28,.37);c.text(40,130+i*68,items[i],28);}c.text(32,430,"Arrows choose  /  A select  /  B return",21);c.text(32,475,"Sonic 1 ROM / Mattyew Sonic 2 casing",19);return;}
  if(!frame->loaded){c.fill(.06,.12,.17);c.text(30,85,"SONIC TIGER LCD",35);c.text(30,165,frame->error,25);c.text(30,230,"Import with tools/import_assets.py",23);c.text(30,285,"Destination: your private data/content",23);c.text(30,390,"Model: Mattyew / CC BY 4.0",22);return;}
  for(auto& s:art)if(frame->segments[s.index]){auto im=s.image;auto surface=cairo_image_surface_create_for_data(im.bits(),CAIRO_FORMAT_ARGB32,im.width(),im.height(),im.bytesPerLine());cairo_set_source_surface(c.c,surface,s.x,s.y);cairo_paint(c.c);cairo_surface_destroy(surface);}
 }
};

#include "archive_emulator.h"
#include "placement.h"
#include <QTemporaryDir>
#include <QPainter>
#include <QLinearGradient>
#include <QFileInfo>
#include <iostream>
static M rotate(float yaw,float pitch){M p=identity();p[5]=cosf(pitch);p[6]=sinf(pitch);p[9]=-sinf(pitch);p[10]=cosf(pitch);M y=identity();y[0]=cosf(yaw);y[2]=-sinf(yaw);y[8]=sinf(yaw);y[10]=cosf(yaw);return mm(y,p);}
int main(int argc,char**argv){try{
 QCoreApplication app(argc,argv);if(argc<3)throw std::runtime_error("Usage: capture_release PACKAGE OUTPUT");
 const std::string package=argv[1],output=argv[2];QDir().mkpath(QString::fromStdString(output));QTemporaryDir data;
 bool existing=argc>3;qputenv("SONIC_NO_AUDIO","1");if(existing)qputenv("SONIC_TEST_OFFLINE","1");
 Emulator game(QString::fromStdString(package),data.path());
 std::unique_ptr<ArchiveMapping> live;std::vector<unsigned char> livePixels(ArchiveWidth*ArchiveHeight*4);
 if(existing&&QString::fromLocal8Bit(argv[3]).endsWith(".png")){
  QImage recorded(QString::fromLocal8Bit(argv[3]));recorded=recorded.convertToFormat(QImage::Format_ARGB32_Premultiplied);
  if(recorded.width()!=ArchiveWidth||recorded.height()!=ArchiveHeight)throw std::runtime_error("Recorded gameplay frame has unexpected dimensions");
  for(int y=0;y<ArchiveHeight;y++)memcpy(livePixels.data()+y*ArchiveWidth*4,recorded.constScanLine(y),ArchiveWidth*4);
 }else if(existing){
  live=std::make_unique<ArchiveMapping>(argv[3],false);bool copied=false;
  for(int attempt=0;attempt<100&&!copied;attempt++){
   auto* s=live->shared;auto seq=s->frameSequence.load(std::memory_order_acquire);
   if(seq&&!(seq&1)){memcpy(livePixels.data(),s->pixels,livePixels.size());copied=s->frameSequence.load(std::memory_order_acquire)==seq;}
   if(!copied)std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }if(!copied)throw std::runtime_error("No completed live frame available");
 }else{
  auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(180);
  while((!game.loaded()||game.ticks<60)&&std::chrono::steady_clock::now()<deadline)std::this_thread::sleep_for(std::chrono::milliseconds(50));
  if(!game.loaded())throw std::runtime_error("Archive player did not load; no placeholder screenshots generated");
  game.press("state");std::this_thread::sleep_for(std::chrono::seconds(3));game.press("state");
  std::this_thread::sleep_for(std::chrono::seconds(3));game.setButtons(4|16);std::this_thread::sleep_for(std::chrono::seconds(2));game.setButtons(0);
 }
 Renderer renderer(package);Canvas lcd(ScreenWidth,ScreenHeight);
 auto capture=[&](const std::string& file,float yaw,float pitch){
  if(existing&&!game.snapshot()->menu){auto surface=cairo_image_surface_create_for_data(livePixels.data(),CAIRO_FORMAT_ARGB32,ArchiveWidth,ArchiveHeight,ArchiveWidth*4);cairo_set_source_surface(lcd.c,surface,0,0);cairo_paint(lcd.c);cairo_surface_destroy(surface);}else game.drawLCD(lcd);
  renderer.screen(lcd);renderer.showGrabHandle=false;
  renderer.draw(identity(),rotate(yaw,pitch),{0,0,.4f},{0,0,.4f},.185f,1);
  auto raw=output+"/raw-"+file;renderer.save(raw);
  QImage device(QString::fromStdString(raw));if(device.isNull())throw std::runtime_error("Cannot read rendered capture");
  QImage image(1024,1024,QImage::Format_ARGB32_Premultiplied);QPainter p(&image);
  QLinearGradient bg(0,0,1024,1024);bg.setColorAt(0,QColor("#17232c"));bg.setColorAt(1,QColor("#080e14"));p.fillRect(image.rect(),bg);p.drawImage(0,0,device);
  p.end();if(!image.save(QString::fromStdString(output+"/"+file)))throw std::runtime_error("Cannot save screenshot");
 };
 capture("sonic-tiger-gameplay.png",-.12f,.055f);
 game.press("menu");std::this_thread::sleep_for(std::chrono::milliseconds(80));
 capture("sonic-tiger-settings.png",-.08f,.035f);
 capture("sonic-tiger-back.png",2.85f,.10f);
 std::cout<<"Captured actual online gameplay, settings and model back; private save and silent audio\n";
 return 0;
}catch(const std::exception&e){std::cerr<<e.what()<<"\n";return 1;}}

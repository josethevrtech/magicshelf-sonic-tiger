#include "archive_emulator.h"
#include <QTemporaryDir>
#include <cassert>
#include <set>
static void waitFor(const std::function<bool()>& condition,int seconds){
 auto end=std::chrono::steady_clock::now()+std::chrono::seconds(seconds);
 while(!condition()&&std::chrono::steady_clock::now()<end)std::this_thread::sleep_for(std::chrono::milliseconds(30));
 assert(condition());
}
int main(int argc,char**argv){
 QCoreApplication app(argc,argv);assert(argc>=3);QString package=argv[1];std::string output=argv[2];
 QTemporaryDir data;
 {
  Emulator game(package,data.path());waitFor([&]{return game.loaded()&&game.ticks>40;},65);
  Canvas lcd(758,540);game.drawLCD(lcd);lcd.png(output+"/online-ready.png");
  auto delivered=game.deliveredEvents.load();
  game.modelDown("back");game.modelUp(); // A tap shorter than either worker tick.
  waitFor([&]{return game.deliveredEvents.load()>=delivered+2&&game.deliveredKeys.load()==0;},3);
  // A held model key must survive controller updates, and release independently.
  game.modelDown("right");game.setButtons(0);waitFor([&]{return game.snapshot()->inputs==4;},2);
  game.setButtons(2);waitFor([&]{return game.snapshot()->inputs==6;},2);
  game.modelUp();waitFor([&]{return game.snapshot()->inputs==2;},2);game.setButtons(0);
  game.modelDown("state");std::this_thread::sleep_for(std::chrono::milliseconds(200));game.modelUp();
  std::this_thread::sleep_for(std::chrono::seconds(3));game.press("state");
  std::this_thread::sleep_for(std::chrono::seconds(2));game.modelDown("right");game.setButtons(16);
  if(!qEnvironmentVariableIsSet("SONIC_NO_AUDIO"))waitFor([&]{return game.audioPeak.load()!=0;},10);
  std::this_thread::sleep_for(std::chrono::seconds(3));game.modelUp();game.setButtons(0);
  game.drawLCD(lcd);lcd.png(output+"/online-playing.png");assert(game.ticks>50);
  game.press("menu");waitFor([&]{return game.snapshot()->menu;},2);game.drawLCD(lcd);lcd.png(output+"/online-settings.png");
  game.press("bottom");game.press("enter");waitFor([&]{return game.snapshot()->mute;},2);
  game.press("back");waitFor([&]{return !game.snapshot()->menu;},2);
  std::cout<<"Archive player loaded without ROM content; "<<game.ticks<<" frames; independent model/controller hold-release and settings passed\n";
 }
 // The supported offline error/retry screen must not require or search for ROMs.
 qputenv("SONIC_TEST_OFFLINE","1");
 {
  Emulator game(package,data.path());waitFor([&]{return game.snapshot()->error.find("Cannot reach")!=std::string::npos;},15);
  assert(!game.loaded());Canvas lcd(758,540);game.drawLCD(lcd);lcd.png(output+"/online-offline.png");
  std::cout<<"Offline error screen passed\n";
 }
}

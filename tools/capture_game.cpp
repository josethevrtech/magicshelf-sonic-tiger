#include "emulator.h"
#include <QTemporaryDir>
int main(int argc,char**argv){QCoreApplication app(argc,argv);QTemporaryDir d;Emulator game(".",d.path());for(int i=0;i<12;i++){if(i==0||i==2)game.press("state");if(i==4)game.setButtons(4|16);if(i==8)game.setButtons(0);std::this_thread::sleep_for(std::chrono::seconds(1));auto f=game.snapshot();std::cout<<i<<":";for(int j=0;j<256;j++)if(f->segments[j])std::cout<<j<<",";std::cout<<"\n";Canvas c(758,540);game.drawLCD(c);c.png("local/lcd-"+std::to_string(i)+".png");}}

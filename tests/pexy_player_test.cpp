#include "archive_shared.h"
#include <QApplication>
#include <QProcess>
#include <QThread>
#include <QImage>
#include <iostream>
int main(int argc,char**argv){QApplication app(argc,argv);ArchiveMapping m(argv[2],true);m.shared->gain=0;QProcess p;p.start(argv[1],{argv[2]});
 auto wait=[&](auto condition,int ms){for(int i=0;i<ms/50&&!condition();i++){app.processEvents();QThread::msleep(50);}return condition();};
 if(!wait([&]{return m.shared->status.load()!=0;},95000)||m.shared->status!=1||!m.shared->verifiedDriver){std::cerr<<p.readAllStandardError().toStdString();p.kill();return 2;}
 wait([&]{return m.shared->frames>30;},5000);
 QImage image(m.shared->pixels,ArchiveWidth,ArchiveHeight,QImage::Format_ARGB32_Premultiplied);image.copy().save(QString(argv[2])+".png");
 for(unsigned key:{128u,16u,32u,64u,1u,2u,4u,8u}){m.shared->keys=key;if(!wait([&]{return m.shared->observedKeys==key;},2000)){std::cerr<<"key failed "<<key<<" observed "<<m.shared->observedKeys<<"\n";p.kill();return 3;}m.shared->keys=0;if(!wait([&]{return m.shared->observedKeys==0;},2000)){p.kill();return 4;}}
 std::cout<<"identity="<<m.shared->verifiedDriver<<" restored="<<m.shared->restored<<" frames="<<m.shared->frames<<" blocked="<<m.shared->blockedRequests<<"\n";
 m.shared->quit=1;p.waitForFinished(5000);std::cout<<"save="<<m.shared->saved<<" ack="<<m.shared->saveAck<<" exit="<<p.exitCode()<<"\n";return m.shared->saveAck?0:5;}

// Hosts the official Archive.org embed. No game files or emulator are bundled.
#include "archive_shared.h"
#include <QApplication>
#include <QWebEngineView>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QWebEngineUrlRequestInterceptor>
#include <QTimer>
#include <QKeyEvent>
#include <QElapsedTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QSaveFile>
#include <iostream>
#include <cstring>
class ArchivePage:public QWebEnginePage {
public:
 using QWebEnginePage::QWebEnginePage;
 void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level,const QString& message,int,const QString&)override{
  if(level==ErrorMessageLevel)std::cerr<<"Archive player: "<<message.toStdString()<<"\n";
 }
};
class OfflineTest:public QWebEngineUrlRequestInterceptor{
public:using QWebEngineUrlRequestInterceptor::QWebEngineUrlRequestInterceptor;
 void interceptRequest(QWebEngineUrlRequestInfo& request)override{request.block(true);}
};
static const char* AudioBridge=R"JS(
(()=>{
 window.__sonicGains=[];window.__sonicVolume=.28;
 const connect=AudioNode.prototype.connect;
 AudioNode.prototype.connect=function(destination,...args){
  if(destination instanceof AudioDestinationNode){
   const gain=this.context.createGain();gain.gain.value=window.__sonicVolume;
   window.__sonicGains.push(gain);connect.call(this,gain);return connect.call(gain,destination,...args);
  }
  return connect.call(this,destination,...args);
 };
 window.__sonicVolumeSet=v=>{window.__sonicVolume=v;for(const g of window.__sonicGains)g.gain.setTargetAtTime(v,g.context.currentTime,.02);};
})()
)JS";
static const char* FitPlayer=R"JS(
(()=>{
 let s=document.getElementById('sonic-fit');if(!s){s=document.createElement('style');s.id='sonic-fit';
 s.textContent='html,body,#wrap,#emulate,#canvasholder{margin:0!important;padding:0!important;position:fixed!important;inset:0!important;width:100vw!important;height:100vh!important;overflow:hidden!important;transform:none!important}canvas{transform:none!important;position:fixed!important;inset:0!important;width:100vw!important;height:100vh!important;margin:0!important}';document.head.append(s);}
 const canvas=document.querySelector('canvas'),start=document.querySelector('.js-emulation-emulate');
 let machine=0;try{const get=globalThis.Module?.__ZN15running_machine30emscripten_get_running_machineEv;if(typeof get==='function')machine=get();}catch{}
 const ready=machine>0&&!!canvas&&canvas.width>=500&&canvas.height>=300&&(!start||getComputedStyle(start).display==='none');
 if(ready){canvas.setAttribute('tabindex','0');canvas.focus();}return ready;
})()
)JS";
int main(int argc,char**argv){try{
 if(argc<2)throw std::runtime_error("Player needs its private buffer path");
 QApplication::setApplicationName("Sonic Tiger LCD Online");QApplication app(argc,argv);
 ArchiveMapping mapping(argv[1],false);auto*shared=mapping.shared;
 QWebEngineProfile profile; // Off-the-record: the official player fetches content each launch.
 profile.setHttpCacheType(QWebEngineProfile::MemoryHttpCache);
 profile.setPersistentCookiesPolicy(QWebEngineProfile::NoPersistentCookies);
 OfflineTest offline;if(qEnvironmentVariableIsSet("SONIC_TEST_OFFLINE"))profile.setUrlRequestInterceptor(&offline);
 QWebEngineView view;view.setPage(new ArchivePage(&profile,&view));view.resize(ArchiveWidth,ArchiveHeight);
 view.settings()->setAttribute(QWebEngineSettings::PlaybackRequiresUserGesture,false);
 QWebEngineScript audio;audio.setName("Sonic volume");audio.setInjectionPoint(QWebEngineScript::DocumentCreation);
 audio.setWorldId(QWebEngineScript::MainWorld);audio.setRunsOnSubFrames(true);audio.setSourceCode(AudioBridge);view.page()->scripts().insert(audio);
 QObject::connect(view.page(),&QWebEnginePage::recentlyAudibleChanged,[&](bool audible){shared->audible=audible;});
 QObject::connect(view.page(),&QWebEnginePage::renderProcessTerminated,[&](auto,int){shared->status=2;});
 bool loaded=false,startClicked=false,domPending=false;unsigned previous=0,reload=shared->reload.load(),lastGain=999;
 QElapsedTimer sinceLoad;sinceLoad.start();
 QObject::connect(view.page(),&QWebEnginePage::loadFinished,[&](bool ok){loaded=ok;sinceLoad.restart();if(!ok)shared->status=2;});
 auto load=[&]{loaded=startClicked=false;previous=0;shared->status=0;sinceLoad.restart();view.load(QUrl("https://archive.org/embed/hh_tsonic"));};
 view.show();load();
 struct Key{unsigned mask;int qt;const char* text;};
 const Key keys[]={{1,Qt::Key_Left,""},{2,Qt::Key_Up,""},{4,Qt::Key_Right,""},{8,Qt::Key_Down,""},
  {16,Qt::Key_Control,""},{32,Qt::Key_5,"5"},{64,Qt::Key_Minus,"-"},{128,Qt::Key_1,"1"},{256,Qt::Key_F2,""},{512,Qt::Key_9,"9"}};
 QTimer timer;QObject::connect(&timer,&QTimer::timeout,[&]{
  if(shared->quit){app.quit();return;}
  if(shared->reload.load()!=reload){reload=shared->reload;load();return;}
  if(loaded&&!startClicked&&sinceLoad.elapsed()>500){startClicked=true;view.page()->runJavaScript("document.cookie='unmute=1;path=/;SameSite=Lax;Secure';document.querySelector('.js-emulation-emulate')?.click()");}
  if(loaded&&!domPending){domPending=true;view.page()->runJavaScript(FitPlayer,[&](QVariant v){domPending=false;if(v.toBool())shared->status=1;});}
  if(sinceLoad.elapsed()>60000&&shared->status==0)shared->status=2;
  unsigned gain=shared->gain.load();if(gain!=lastGain){lastGain=gain;view.page()->setAudioMuted(gain==0);}
  view.page()->runJavaScript(QString("window.__sonicVolumeSet?.(%1)").arg(gain/100.0));
  unsigned input=shared->status==1?shared->keys.load():0;
  auto* target=view.focusProxy();if(!target)target=&view;
  for(auto& key:keys)if((input^previous)&key.mask){QKeyEvent event(input&key.mask?QEvent::KeyPress:QEvent::KeyRelease,key.qt,input&16?Qt::ControlModifier:Qt::NoModifier,QString::fromLatin1(key.text));QApplication::sendEvent(target,&event);shared->deliveredEvents.fetch_add(1);}
  shared->deliveredKeys=input;
  previous=input;
  if(shared->status==1){
   QImage image=view.grab().toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
   if(image.width()!=ArchiveWidth||image.height()!=ArchiveHeight)image=image.scaled(ArchiveWidth,ArchiveHeight);
   shared->frameSequence.fetch_add(1,std::memory_order_acq_rel);
   for(int y=0;y<ArchiveHeight;y++)memcpy(shared->pixels+y*ArchiveWidth*4,image.constScanLine(y),ArchiveWidth*4);
   shared->frameSequence.fetch_add(1,std::memory_order_release);shared->frames.fetch_add(1);
  }
 });timer.start(50);
 return app.exec();
}catch(const std::exception&e){std::cerr<<e.what()<<"\n";return 1;}}

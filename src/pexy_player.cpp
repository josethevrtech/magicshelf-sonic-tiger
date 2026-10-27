// Hosts the site's actual Sonic 2 player, not a local ROM or a Sonic 1 fallback.
#include "archive_shared.h"
#include "player_policy.h"
#include <QApplication>
#include <QWebEngineView>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QWebEngineUrlRequestInterceptor>
#include <QWebEngineUrlRequestInfo>
#include <QWebEngineDownloadRequest>
#include <QFileInfo>
#include <QDir>
#include <QTimer>
#include <QKeyEvent>
#include <QElapsedTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
#include <cstring>

class PlayerPage:public QWebEnginePage {
public:using QWebEnginePage::QWebEnginePage;
 bool acceptNavigationRequest(const QUrl& url,NavigationType,bool main)override{return !main||playerNavigationAllowed(url);}
 QWebEnginePage* createWindow(WebWindowType)override{return nullptr;}
 void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level,const QString& message,int,const QString&)override{
  if(level==ErrorMessageLevel)std::cerr<<"Sonic 2 player: "<<message.toStdString()<<"\n";
 }
};
class PlayerRequests:public QWebEngineUrlRequestInterceptor {
 ArchiveShared* shared;bool offline;
public:PlayerRequests(ArchiveShared* s):shared(s),offline(qEnvironmentVariableIsSet("SONIC_TEST_OFFLINE")){}
 void interceptRequest(QWebEngineUrlRequestInfo& request)override{
  if(offline||!playerRequestAllowed(request.requestUrl())){shared->blockedRequests++;request.block(true);}
 }
};
static const char* AudioBridge=R"JS(
(()=>{
 const gains=new Set();window.__sonicVolume=.28;
 const connect=AudioNode.prototype.connect;
 AudioNode.prototype.connect=function(destination,...args){
  if(destination instanceof AudioDestinationNode){
   const gain=this.context.createGain();gain.gain.value=window.__sonicVolume;gains.add(gain);
   // This player creates a new BufferSource for every audio chunk. Retire its
   // gain on completion instead of retaining thousands of disconnected nodes.
   if('onended' in this)this.addEventListener('ended',()=>{gains.delete(gain);gain.disconnect();},{once:true});
   connect.call(this,gain);return connect.call(gain,destination,...args);
  }return connect.call(this,destination,...args);
 };
 window.__sonicVolumeSet=v=>{window.__sonicVolume=v;for(const g of gains)g.gain.setTargetAtTime(v,g.context.currentTime,.02);};
 const masks={ArrowLeft:1,ArrowUp:2,ArrowRight:4,ArrowDown:8,Space:16,KeyT:32,KeyN:64,Digit1:128,Digit0:256,KeyR:512};
 window.__sonicObserved=0;
 window.addEventListener('keydown',e=>{window.__sonicObserved|=masks[e.code]||0;},true);
 window.addEventListener('keyup',e=>{window.__sonicObserved&=~(masks[e.code]||0);},true);
})()
)JS";
static const char* InspectAndFit=R"JS(
(()=>{
 const player=window.pexyArcade,canvas=document.getElementById('canvas');
 const args=player?.args||[];
 const identity=args.includes('tsonic2')&&!args.includes('tsonic')&&document.title.includes('[tsonic2]');
 const clean=Array.isArray(player?.romProblems)&&player.romProblems.length===0;
 const ready=identity&&clean&&player?.state?.phase==='running'&&window.JSMESS?.running&&canvas?.width>0;
 if(ready){
  let style=document.getElementById('sonic-fit');if(!style){style=document.createElement('style');style.id='sonic-fit';
   style.textContent='html,body,#stage,#device,#window{position:fixed!important;inset:0!important;margin:0!important;padding:0!important;transform:none!important;width:100vw!important;height:100vh!important;overflow:hidden!important;box-shadow:none!important;border:0!important;border-radius:0!important}#canvas{position:fixed!important;inset:0!important;width:100vw!important;height:100vh!important;margin:0!important;transform:none!important}#bar,#pad,#help,#device>.case,#device>.hot,#device>.label,#device>.brand,#device>.title{display:none!important}';document.head.appendChild(style);}
  canvas.setAttribute('tabindex','0');canvas.focus();
 }
 let save=null;try{save=JSON.parse(localStorage.getItem('pexy-lcd:tsonic2')||'null');}catch{}
 return JSON.stringify({ready,identity,error:player?.state?.phase==='error',saved:!!save?.data,restored:!!player?.state?.restored,
  saveAck:window.__sonicSaveAck||0,keys:window.__sonicObserved||0});
})()
)JS";

int main(int argc,char**argv){try{
 if(argc<2)throw std::runtime_error("Player needs its private buffer path");
 QApplication::setApplicationName("Sonic 2 Tiger LCD");QApplication app(argc,argv);
 ArchiveMapping mapping(argv[1],false);auto*shared=mapping.shared;
 const QString storage=QFileInfo(QString::fromLocal8Bit(argv[1])).absolutePath()+"/sonic2-browser";QDir().mkpath(storage);
 QWebEngineProfile profile("magicshelf-sonic2");profile.setPersistentStoragePath(storage);
 profile.setHttpCacheType(QWebEngineProfile::MemoryHttpCache);profile.setPersistentCookiesPolicy(QWebEngineProfile::NoPersistentCookies);
 PlayerRequests requests(shared);profile.setUrlRequestInterceptor(&requests);
 QObject::connect(&profile,&QWebEngineProfile::downloadRequested,[](QWebEngineDownloadRequest* download){download->cancel();});
 QWebEngineView view;view.setPage(new PlayerPage(&profile,&view));view.resize(ArchiveWidth,ArchiveHeight);
 view.settings()->setAttribute(QWebEngineSettings::PlaybackRequiresUserGesture,false);
 QObject::connect(view.page(),&QWebEnginePage::featurePermissionRequested,[&](const QUrl& origin,QWebEnginePage::Feature feature){view.page()->setFeaturePermission(origin,feature,QWebEnginePage::PermissionDeniedByUser);});
 QWebEngineScript audio;audio.setName("Sonic volume and input diagnostics");audio.setInjectionPoint(QWebEngineScript::DocumentCreation);
 audio.setWorldId(QWebEngineScript::MainWorld);audio.setSourceCode(AudioBridge);view.page()->scripts().insert(audio);
 QObject::connect(view.page(),&QWebEnginePage::recentlyAudibleChanged,[&](bool audible){shared->audible=audible;});
 QObject::connect(view.page(),&QWebEnginePage::renderProcessTerminated,[&](auto,int){shared->status=2;});
 bool loaded=false,domPending=false;unsigned previous=0,reload=shared->reload.load(),operation=0,saveSequence=0;
 QElapsedTimer sinceLoad,operationTime;sinceLoad.start();
 QObject::connect(view.page(),&QWebEnginePage::loadFinished,[&](bool ok){loaded=ok;sinceLoad.restart();if(!ok)shared->status=2;});
 auto load=[&]{loaded=false;previous=0;shared->status=0;shared->verifiedDriver=0;shared->restored=0;sinceLoad.restart();view.load(QUrl(PlayerEmbedUrl));};
 auto startOperation=[&](unsigned kind){
  operation=kind;operationTime.restart();view.page()->setAudioMuted(true);
  ++saveSequence;
  view.page()->runJavaScript(QString(R"JS((()=>{const seq=%1,at=Date.now();window.pexyArcade?.saveState?.();setTimeout(()=>{try{const s=JSON.parse(localStorage.getItem('pexy-lcd:tsonic2')||'null');if(s?.data&&s.t>=at)window.__sonicSaveAck=seq;}catch{}},700);})())JS").arg(saveSequence));
 };
 view.show();load();
 struct Key{unsigned mask;int qt;const char* text;};
 const Key keys[]={{1,Qt::Key_Left,""},{2,Qt::Key_Up,""},{4,Qt::Key_Right,""},{8,Qt::Key_Down,""},
  {16,Qt::Key_Space," "},{32,Qt::Key_T,"t"},{64,Qt::Key_N,"n"},{128,Qt::Key_1,"1"},{256,Qt::Key_0,"0"},{512,Qt::Key_R,"r"}};
 QTimer timer;QObject::connect(&timer,&QTimer::timeout,[&]{
  if(!operation&&shared->quit)startOperation(2);
  if(!operation&&shared->reload.load()!=reload){reload=shared->reload;startOperation(1);}
  if(operation&&((operationTime.elapsed()>800&&shared->saveAck.load()==saveSequence)||operationTime.elapsed()>1800)){
   if(operation==2){app.quit();return;}operation=0;load();return;
  }
  if(loaded&&!domPending){domPending=true;view.page()->runJavaScript(InspectAndFit,[&](QVariant value){
   domPending=false;auto state=QJsonDocument::fromJson(value.toString().toUtf8()).object();
   shared->verifiedDriver=state["identity"].toBool();shared->saved=state["saved"].toBool();shared->restored=state["restored"].toBool();
   shared->saveAck=state["saveAck"].toInt();shared->observedKeys=state["keys"].toInt();
   if(state["ready"].toBool())shared->status=1;else if(state["error"].toBool())shared->status=2;
  });}
  if(sinceLoad.elapsed()>90000&&shared->status==0)shared->status=2;
  unsigned gain=operation?0:shared->gain.load();view.page()->setAudioMuted(gain==0);
  view.page()->runJavaScript(QString("window.__sonicVolumeSet?.(%1)").arg(gain/100.0));
  unsigned input=shared->status==1&&!operation?shared->keys.load():0;auto* target=view.focusProxy();if(!target)target=&view;
  for(auto& key:keys)if((input^previous)&key.mask){QKeyEvent event(input&key.mask?QEvent::KeyPress:QEvent::KeyRelease,key.qt,Qt::NoModifier,QString::fromLatin1(key.text));QApplication::sendEvent(target,&event);shared->deliveredEvents++;}
  shared->deliveredKeys=input;previous=input;
  if(shared->status==1&&!operation){QImage image=view.grab().toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
   if(image.size()!=QSize(ArchiveWidth,ArchiveHeight))image=image.scaled(ArchiveWidth,ArchiveHeight);
   shared->frameSequence.fetch_add(1,std::memory_order_acq_rel);
   for(int y=0;y<ArchiveHeight;y++)memcpy(shared->pixels+y*ArchiveWidth*4,image.constScanLine(y),ArchiveWidth*4);
   shared->frameSequence.fetch_add(1,std::memory_order_release);shared->frames++;
  }
 });timer.start(50);return app.exec();
}catch(const std::exception&e){std::cerr<<e.what()<<"\n";return 1;}}

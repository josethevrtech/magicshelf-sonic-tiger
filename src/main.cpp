#ifdef SONIC_ARCHIVE_PLAYER
#include "archive_emulator.h"
#else
#include "emulator.h"
#endif
#include "magicshelf_sdk.h"
#include "controls.h"
#include "controller_nav.h"
#include "shelf_fit.h"
#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QTimer>
#include <QTemporaryDir>
#include <QJsonArray>
#include <chrono>
#include <csignal>
#include <thread>
#include <sys/file.h>
#include <fcntl.h>
#include <unistd.h>
namespace fs=std::filesystem;
using namespace vr;
static volatile sig_atomic_t running=1,showRequested=0,dockRequested=0;
static void stop(int){running=0;}static void show(int){showRequested=1;}static void home(int){dockRequested=1;}
static M fromvr(HmdMatrix34_t p){M r=identity();for(int i=0;i<3;i++)for(int j=0;j<4;j++)r[i+j*4]=p.m[i][j];return r;}
static void check(EVROverlayError e){if(e)throw std::runtime_error(VROverlay()->GetOverlayErrorNameFromEnum(e));}
static M rotated(float yaw,float pitch){M p=identity();p[5]=cosf(pitch);p[6]=sinf(pitch);p[9]=-sinf(pitch);p[10]=cosf(pitch);return mm(yawPose(yaw),p);}

class Desktop : public QWidget {
    Emulator& game;Renderer& renderer;Canvas lcd{ScreenWidth,ScreenHeight};QTimer timer;
    QPoint drag;bool dragging=false;float yaw=-.15f,pitch=.06f;int saves=0;
public:
    Desktop(Emulator& g,Renderer& r):game(g),renderer(r){
        setWindowTitle("Sonic Tiger LCD · MagicShelf");resize(720,790);setMinimumSize(480,550);setFocusPolicy(Qt::StrongFocus);
        connect(&timer,&QTimer::timeout,this,[&]{try{if(++saves%30==0)game.save();update();if(!running)close();}catch(const std::exception&e){std::cerr<<e.what()<<"\n";timer.stop();close();}});timer.start(33);
    }
    void paintEvent(QPaintEvent*)override{
        eglMakeCurrent(renderer.display,renderer.surface,renderer.surface,renderer.context);
        game.drawLCD(lcd);renderer.screen(lcd);
        auto*raw=renderer.draw(identity(),rotated(yaw,pitch),{0,0,.3f},{0,0,.3f},.19f,1);
        QPainter p(this);p.fillRect(rect(),QColor("#eee8d8"));
        int side=std::min(width(),height()-100);QImage image(raw,renderer.W,renderer.H,renderer.W*4,QImage::Format_RGBA8888);
        p.drawImage(QRect((width()-side)/2,20,side,side),image,QRect(0,0,renderer.W/2,renderer.H));
        p.setPen(QColor("#514a38"));p.setFont(QFont("sans",12));
        p.drawText(QRect(12,height()-77,width()-24,24),Qt::AlignCenter,"Arrows: move/jump    Enter: spin    Backspace: pause");
        p.drawText(QRect(12,height()-48,width()-24,24),Qt::AlignCenter,"F: power on/start    M: settings    Drag: rotate");
    }
    void keyPressEvent(QKeyEvent* e)override {
        if(e->isAutoRepeat())return;QString button;
        switch(e->key()){
            case Qt::Key_Return:case Qt::Key_Enter:button="enter";break;case Qt::Key_Backspace:case Qt::Key_Escape:button="back";break;
            case Qt::Key_Left:button="left";break;case Qt::Key_Right:button="right";break;case Qt::Key_Up:button="top";break;case Qt::Key_Down:button="bottom";break;
            case Qt::Key_F:button="state";break;case Qt::Key_M:button="menu";break;case Qt::Key_Space:button="shake";break;
            case Qt::Key_R:if(e->modifiers()&Qt::ShiftModifier)button="reset";break;
        }if(!button.isEmpty()){game.key(button,true);update();}
    }
    void keyReleaseEvent(QKeyEvent* e)override{if(e->isAutoRepeat())return;QString b;switch(e->key()){case Qt::Key_Left:b="left";break;case Qt::Key_Right:b="right";break;case Qt::Key_Up:b="top";break;case Qt::Key_Down:b="bottom";break;case Qt::Key_Return:case Qt::Key_Enter:b="enter";break;case Qt::Key_Backspace:b="back";break;}if(!b.isEmpty())game.key(b,false);}
    void focusOutEvent(QFocusEvent*)override{game.setButtons(0);game.modelUp();}
    void mousePressEvent(QMouseEvent* e)override{
        int side=std::min(width(),height()-100);V eye{0,0,.3f};
        V target{float((e->position().x()-(width()-side)/2)/side-.5)*.19f,float(.5-(e->position().y()-20)/side)*.19f,0};
        M inv=inverse(rotated(yaw,pitch));V a=point(inv,eye),b=point(inv,target),d=sub(b,a);
        auto button=buttonRay(a,d);if(!button.empty()){game.modelDown(QString::fromStdString(button));return;}
        dragging=true;drag=e->position().toPoint();
    }
    void mouseMoveEvent(QMouseEvent* e)override{if(dragging){auto delta=e->position().toPoint()-drag;drag=e->position().toPoint();yaw+=delta.x()*.012f;pitch=std::clamp(pitch+delta.y()*.012f,-1.2f,1.2f);update();}}
    void mouseReleaseEvent(QMouseEvent*)override{dragging=false;game.modelUp();}
};

static int runVR(Emulator& game,Renderer& renderer,const std::string& package,const std::string& data,int lock){
    EVRInitError err;VR_Init(&err,VRApplication_Overlay);if(err)throw std::runtime_error(VR_GetVRInitErrorAsEnglishDescription(err));
    auto*ov=VROverlay();VROverlayHandle_t device=0;
    check(ov->CreateOverlay(getenv("MAGICSHELF_MODULE_KEY")?getenv("MAGICSHELF_MODULE_KEY"):DeviceKey,AppTitle,&device));
    for(auto flag:{VROverlayFlags_SideBySide_Parallel,VROverlayFlags_VisibleInDashboard,VROverlayFlags_MakeOverlaysInteractiveIfVisible,VROverlayFlags_EnableClickStabilization,VROverlayFlags_SendVRSmoothScrollEvents})check(ov->SetOverlayFlag(device,flag,true));
    check(ov->SetOverlayInputMethod(device,VROverlayInputMethod_Mouse));HmdVector2_t mouse{{512,512}};check(ov->SetOverlayMouseScale(device,&mouse));
    ObjectDock dock(ov,data);ControllerNav nav(package);RoomGrab grab;ShakeToDock shake;
    Canvas lcd(ScreenWidth,ScreenHeight);M world=identity(),lastPlane=identity(),lastModel=identity();
    float size=DefaultSize,lastSize=1,lastWidth=InteractionPlaneWidth;bool docked=true,placed=false,ready=false,candidate=false;
    uint32_t pointer=k_unTrackedDeviceIndexInvalid;bool overHandle=false;auto now=std::chrono::steady_clock::now(),last=now,lastSave=now,lastLCD=now-std::chrono::seconds(1);
    double frameMs=0,peakFrameMs=0;unsigned long frameCount=0,slowFrames=0,syncErrors=0;bool frameSync=false;
    unsigned long controllerInputs=0,depthMoves=0;
    bool gripHeld=false;bool repositioning=false,startupPlacement=true;auto placementStarted=now;
    uint32_t modelButtonController=k_unTrackedDeviceIndexInvalid;std::string modelButton;
    unsigned long modelButtonPresses=0,modelPointerClicks=0;std::string hoveredModelButton;V lastModelHit{};
    auto clearModelButton=[&]{modelButtonController=k_unTrackedDeviceIndexInvalid;modelButton.clear();game.modelUp();};
    auto settings=data+"/placement.json";
    if(auto*j=json_object_from_file(settings.c_str())){
        json_object*v=nullptr;
        if(json_object_object_get_ex(j,"size",&v)){float value=json_object_get_double(v);if(std::isfinite(value))size=std::clamp(value,.6f,3.f);}
        if(json_object_object_get_ex(j,"docked",&v))docked=json_object_get_boolean(v);
        if(json_object_object_get_ex(j,"pose",&v)&&json_object_is_type(v,json_type_array)&&json_object_array_length(v)==16){for(int i=0;i<16;i++)world[i]=json_object_get_double(json_object_array_get_idx(v,i));placed=validRoomPose(world);}
        json_object_put(j);
    }
    auto savePlacement=[&]{QJsonArray pose;for(float v:world)pose.append(v);QJsonObject j{{"size",size},{"docked",docked},{"pose",pose}};QSaveFile f(QString::fromStdString(settings));if(!f.open(QIODevice::WriteOnly))throw std::runtime_error("Cannot save placement");f.write(QJsonDocument(j).toJson());if(!f.commit())throw std::runtime_error("Cannot save placement");};
    auto toDock=[&]{clearModelButton();startupPlacement=true;placementStarted=std::chrono::steady_clock::now();showRequested=0;grab.end();shake.reset();nav.state.update({},0);gripHeld=false;docked=true;placed=false;candidate=false;savePlacement();};
    auto release=[&]{if(dock.canSnap(world,size,candidate))toDock();else{grab.end();shake.reset();candidate=false;savePlacement();}};
    while(running){
        now=std::chrono::steady_clock::now();float dt=std::chrono::duration<float>(now-last).count();last=now;frameMs=dt*1000;peakFrameMs=std::max(peakFrameMs,frameMs);++frameCount;if(frameMs>33.34)++slowFrames;

        TrackedDevicePose_t poses[k_unMaxTrackedDeviceCount];VRSystem()->GetDeviceToAbsoluteTrackingPose(TrackingUniverseStanding,0,poses,k_unMaxTrackedDeviceCount);
        M head=fromvr(poses[0].mDeviceToAbsoluteTracking);dock.refresh();
        if(dockRequested||dock.takeHomeRequest()){dockRequested=0;toDock();}
        if(startupPlacement&&poses[0].bPoseIsValid&&(!docked||dock.available))startupPlacement=false;
        bool missingDock=startupPlacement&&docked&&!dock.available&&now-placementStarted>std::chrono::seconds(1);
        if(poses[0].bPoseIsValid&&(showRequested||missingDock||(!placed&&!docked))){
            startupPlacement=false;
            showRequested=0;grab.end();docked=false;placed=true;int columns=std::clamp(dock.layout.count,1,4);float offsetX=missingDock&&dock.slot>=0?(dock.slot%columns-(columns-1)*.5f)*.24f:-.08f;float offsetY=-.10f-(missingDock&&dock.slot>=0?dock.slot/columns*.24f:0);V p=point(head,{offsetX,offsetY,-.60f}),h=point(head,{0,0,0}),z=norm(sub(h,p)),x=norm(cross({0,1,0},z));world=pose(x,cross(z,x),z,p);savePlacement();
        }
        if(!grab.active)gripHeld=false;
  bool resetTracking=false;
        VREvent_t e;while(VRSystem()->PollNextEvent(&e,sizeof(e))){if(e.eventType==VREvent_Quit){VRSystem()->AcknowledgeQuit_Exiting();running=0;}if(dockResetEvent(e.eventType)){toDock();resetTracking=true;}}
        if(now-lastSave>std::chrono::seconds(1)){
            game.save();lastSave=now;
            QJsonObject status{{"frame_ms",frameMs},{"peak_frame_ms",peakFrameMs},{"frame_count",double(frameCount)},
                {"slow_frames",double(slowFrames)},{"frame_sync",frameSync},{"held",grab.active},{"docked",docked},
                {"emulator_ticks",double(game.ticks.load())},{"rom_loaded",game.loaded()},{"controller_inputs",double(controllerInputs)},
                {"headset_tracked",poses[0].bPoseIsValid},{"overlay_visible",ov->IsOverlayVisible(device)},
                {"audio_connected",game.audioConnected.load()},{"audio_samples",double(game.audioSamples.load())},{"audio_peak",game.audioPeak.load()},
                {"input_focused",nav.focusedNow},{"input_fallback",nav.usingFallback},{"input_pressed_mask",int(nav.pressedMask)},
                {"model_button",QString::fromStdString(modelButton)},{"model_button_presses",double(modelButtonPresses)}};
            status["model_pointer_clicks"]=double(modelPointerClicks);status["model_hover"]=QString::fromStdString(hoveredModelButton);
            status["model_hit_x"]=lastModelHit[0];status["model_hit_y"]=lastModelHit[1];
            status["display_scale"]=lastSize;status["user_scale"]=size;
            status["dock_spacing_m"]=dock.effective.spacing;
            status["body_width_m"]=TigerSlotWidth*lastSize;
#ifdef SONIC_ARCHIVE_PLAYER
            status["player_input_mask"]=int(game.deliveredKeys.load());status["player_input_events"]=int(game.deliveredEvents.load());
            status["player_ready"]=game.loaded();status["player_frames"]=double(game.ticks.load());status["audio_recently_audible"]=bool(game.audioPeak.load());
            status.remove("emulator_ticks");status.remove("rom_loaded");status.remove("audio_peak");status.remove("audio_samples");
#endif
            std::ofstream f(data+"/runtime.json.next");f<<QJsonDocument(status).toJson().constData();f.close();
            if(f){std::error_code ec;fs::rename(data+"/runtime.json.next",data+"/runtime.json",ec);}
        }
        if(grab.active){
            if(!poses[grab.controller].bPoseIsValid||!poses[0].bPoseIsValid)release();
            else if(grab.watchTriggerRelease){
                auto role=int(VRSystem()->GetControllerRoleForTrackedDeviceIndex(grab.controller));
                RenderModel_ComponentState_t trigger{};
                if(role>=1&&role<=2&&nav.frame.component(role,nav.hand[role-1],"trigger",trigger)){
                    if(!(trigger.uProperties&VRComponentProperty_IsPressed))release();
                }else{VRControllerState_t state{};if(VRSystem()->GetControllerState(grab.controller,&state,sizeof(state))&&!(state.ulButtonPressed&ButtonMaskFromId(k_EButton_SteamVR_Trigger)))release();}
            }
        }
        if(modelButtonController<k_unMaxTrackedDeviceCount){
            if(!poses[0].bPoseIsValid||!poses[modelButtonController].bPoseIsValid)clearModelButton();
            // Overlay mouse-up is authoritative. The render-model trigger has
            // a different press threshold and can report released during clicks.
        }
        while(ov->PollNextOverlayEvent(device,&e,sizeof(e))){
            if(dockResetEvent(e.eventType)){toDock();continue;}
            if(e.eventType==VREvent_FocusLeave)clearModelButton();
            if(e.eventType==VREvent_MouseButtonUp&&e.data.mouse.button==VRMouseButton_Left&&
                (e.trackedDeviceIndex==modelButtonController||e.trackedDeviceIndex==k_unTrackedDeviceIndexInvalid||e.trackedDeviceIndex==0))clearModelButton();
            if(e.eventType==VREvent_ScrollSmooth&&grab.active){
                // Stick input is routed through ControllerNav, including its raw
                // Frame fallback. Scroll must not also change placement.
                continue;
            }
            if(e.eventType==VREvent_MouseButtonUp&&grab.active&&!gripHeld&&!grab.watchTriggerRelease&&(e.trackedDeviceIndex==grab.controller||e.trackedDeviceIndex==k_unTrackedDeviceIndexInvalid||e.trackedDeviceIndex==0)){release();continue;}
            if(!poses[0].bPoseIsValid||(e.eventType!=VREvent_MouseMove&&e.eventType!=VREvent_MouseButtonDown))continue;
            if(grab.active&&e.trackedDeviceIndex==grab.controller)continue;
            if(e.eventType==VREvent_MouseButtonDown&&e.data.mouse.button!=VRMouseButton_Left)continue;
            auto index=e.trackedDeviceIndex;if(index==0||index>=k_unMaxTrackedDeviceCount||!poses[index].bPoseIsValid)index=ov->GetPrimaryDashboardDevice();if(index>=k_unMaxTrackedDeviceCount||!poses[index].bPoseIsValid)continue;
            V origin=point(fromvr(poses[index].mDeviceToAbsoluteTracking),{0,0,0});
            V target=point(lastPlane,{(e.data.mouse.x/512-.5f)*lastWidth,(e.data.mouse.y/512-.5f)*lastWidth,0});
            M inv=inverse(lastModel);V a=mul(point(inv,origin),1/lastSize),b=mul(point(inv,target),1/lastSize),delta=sub(b,a);
            if(fabsf(delta[2])<.0001f)continue;float t=(ScreenZ-a[2])/delta[2];if(t<0)continue;V hit=add(a,mul(delta,t));
            bool front=a[2]>ScreenZ;pointer=index;overHandle=onGrabHandle(hit);
            auto button=buttonRay(a,delta);
            hoveredModelButton=button;lastModelHit=hit;
            if(e.eventType==VREvent_MouseMove){
                if(index==modelButtonController&&button!=modelButton)clearModelButton();
                continue;
            }
            ++modelPointerClicks;
            if(!button.empty()){clearModelButton();modelButtonController=index;modelButton=button;++modelButtonPresses;game.modelDown(QString::fromStdString(button));ov->TriggerLaserMouseHapticVibration(device,.025f,80,.18f);continue;}
            if(grab.active||(front&&onGameScreen(hit)))continue;
            if(overHandle||onPhoneBody(hit)){
                V grabbed=point(lastModel,mul(hit,lastSize));
                if(docked){world=lastModel;V restored=point(world,mul(hit,size));for(int i=0;i<3;i++)world[12+i]+=grabbed[i]-restored[i];docked=false;placed=true;}
                VRControllerState_t state{};bool watch=VRSystem()->GetControllerState(index,&state,sizeof(state))&&(state.ulButtonPressed&ButtonMaskFromId(k_EButton_SteamVR_Trigger));
                int role=int(VRSystem()->GetControllerRoleForTrackedDeviceIndex(index));RenderModel_ComponentState_t trigger{};
                if(role>=1&&role<=2&&nav.frame.component(role,nav.hand[role-1],"trigger",trigger))watch=trigger.uProperties&VRComponentProperty_IsPressed;
                shake.reset();grab.beginAt(index,fromvr(poses[index].mDeviceToAbsoluteTracking),world,grabbed,watch);ov->TriggerLaserMouseHapticVibration(device,.03f,80,.2f);
            }
        }
        uint32_t controller=grab.active?grab.controller:pointer;bool nearHand=false;
        if(!grab.active)for(auto handRole:{TrackedControllerRole_LeftHand,TrackedControllerRole_RightHand}){auto hand=VRSystem()->GetTrackedDeviceIndexForControllerRole(handRole);if(hand>=k_unMaxTrackedDeviceCount||!poses[hand].bPoseIsValid)continue;V local=mul(point(inverse(lastModel),point(fromvr(poses[hand].mDeviceToAbsoluteTracking),{0,0,0})),1/lastSize);if(std::abs(local[0])<BodyX+.07f&&std::abs(local[1])<BodyY+.07f&&std::abs(local[2]-ScreenZ)<.10f){controller=hand;nearHand=true;break;}}
        bool focused=(grab.active||(docked?dock.available:placed))&&controller<k_unMaxTrackedDeviceCount&&poses[controller].bPoseIsValid&&poses[0].bPoseIsValid&&(grab.active||nearHand||ov->IsHoverTargetOverlay(device));
        int role=focused?int(VRSystem()->GetControllerRoleForTrackedDeviceIndex(controller)):0;
        auto action=nav.update(focused,role,dt,grab.active,controller);repositioning=false;
        bool wasHeld=grab.active;
        if(grab.active&&gripHeld&&(!action.gripAvailable||!action.grip)){release();gripHeld=false;}
        if(!grab.active&&focused&&action.gripPressed){
            V grabbed=point(lastModel,{0,0,0});if(docked){world=lastModel;docked=false;placed=true;}
            shake.reset();grab.beginAt(controller,fromvr(poses[controller].mDeviceToAbsoluteTracking),world,grabbed,false);gripHeld=true;
            ov->TriggerLaserMouseHapticVibration(device,.035f,80,.25f);
        }
        bool inputTransition=false;
        if(grab.active){float before=grab.distance;grab.updateDepth(action.depthY,dt,true);repositioning=std::abs(grab.distance-before)>.00001f;if(repositioning)++depthMoves;world=grab.follow(fromvr(poses[controller].mDeviceToAbsoluteTracking));candidate=dock.canSnap(world,size,candidate);}
        if(grab.active){
            M hand=fromvr(poses[grab.controller].mDeviceToAbsoluteTracking);
            if(shake.update(sub(point(hand,{0,0,-.15f}),point(head,{0,0,0})),std::chrono::duration<double>(now.time_since_epoch()).count())){
                ov->TriggerLaserMouseHapticVibration(device,.09f,100,.5f);toDock();inputTransition=true;
            }
        }else shake.reset();
        if(focused&&!inputTransition&&wasHeld==grab.active){
            if(action.home){toDock();game.setButtons(0);continue;}
            if(action.friendship)game.press("state");if(action.settings)game.press("menu");
            auto heldInput=nav.state.previous;
            if(game.snapshot()->menu){
                game.setButtons(0);if(action.confirm)game.press("enter");if(action.back)game.press("back");
                if(action.x)game.press(action.x>0?"right":"left");if(action.y)game.press(action.y>0?"top":"bottom");
            }else{
                unsigned keys=(heldInput.left?1:0)|(heldInput.up?2:0)|(heldInput.right?4:0)|(heldInput.down?8:0)|
                    (heldInput.confirm?16:0)|(heldInput.back?32:0);
                if(heldInput.axis){if(heldInput.x<-.55f)keys|=1;if(heldInput.x>.55f)keys|=4;if(heldInput.y>.55f)keys|=2;if(heldInput.y<-.55f)keys|=8;}
                game.setButtons(keys);if(keys)++controllerInputs;
            }
        }else game.setButtons(0);
        bool visible=poses[0].bPoseIsValid&&(docked?dock.available:placed);
        bool capture=fs::exists(data+"/capture.request");
        if((visible&&now-lastLCD>std::chrono::milliseconds(66))||capture){game.drawLCD(lcd);renderer.screen(lcd);lastLCD=now;}
        if(capture){renderer.showGrabHandle=false;renderer.draw(identity(),rotated(-.18f,.05f),{0,0,.3f},{0,0,.3f},.19f,1);renderer.save(data+"/capture.png");fs::remove(data+"/capture.request");}
        renderer.showGrabHandle=true;renderer.highlightGrabHandle=grab.active||overHandle;renderer.dockGrabHandle=candidate;
        if(poses[0].bPoseIsValid&&(docked?dock.available:placed)){
            auto fitted=tigerPlacement(dock,size,docked,world);
            M model=fitted.model;float effective=fitted.scale;
            V p=point(model,{0,0,ScreenZ*effective}),h=point(head,{0,0,0}),z=norm(sub(h,p)),x=norm(cross({0,1,0},z));M plane=pose(x,cross(z,x),z,p);float width=InteractionPlaneWidth*effective;
            if(dot(sub(h,p),sub(h,p))>.015f){
                V eyes[2];for(int i=0;i<2;i++)eyes[i]=point(mm(head,fromvr(VRSystem()->GetEyeToHeadTransform((EVREye)i))),{0,0,0});renderer.draw(plane,model,eyes[0],eyes[1],width,effective,false);glFinish();Texture_t texture{(void*)(uintptr_t)renderer.tex,TextureType_OpenGL,ColorSpace_Gamma};check(ov->SetOverlayTexture(device,&texture));check(ov->SetOverlayWidthInMeters(device,width));auto transform=magicshelfTransform(plane);check(ov->SetOverlayTransformAbsolute(device,TrackingUniverseStanding,&transform));check(ov->ShowOverlay(device));lastPlane=plane;lastModel=model;lastSize=effective;lastWidth=width;
            }else ov->HideOverlay(device);
        }else ov->HideOverlay(device);
        if(!ready){std::string pid=std::to_string(getpid());if(ftruncate(lock,0)||pwrite(lock,pid.data(),pid.size(),0)!=ssize_t(pid.size()))throw std::runtime_error("Cannot record instance PID");game.save();if(getenv("MAGICSHELF_SESSION"))magicshelfReady();ready=true;std::cerr<<"Sonic Tiger LCD creation ready\n";}
        // Sample dock/head/controller transforms at the runtime frame boundary,
        // rather than drifting through a fixed sleep plus render time.
        frameSync=ov->WaitFrameSync(20)==VROverlayError_None;
        if(!frameSync){++syncErrors;std::this_thread::sleep_until(now+std::chrono::milliseconds(11));}
    }
    game.save();savePlacement();ov->DestroyOverlay(device);VR_Shutdown();return 0;
}
int main(int argc,char**argv){try{
    const std::string mode=argc>1?argv[1]:"";std::unique_ptr<QCoreApplication> app;
    if(mode=="--desktop")app=std::make_unique<QApplication>(argc,argv);else app=std::make_unique<QCoreApplication>(argc,argv);
    std::string package=fs::canonical("/proc/self/exe").parent_path().parent_path().string();
    std::string data=getenv("MAGICSHELF_DATA_DIR")?getenv("MAGICSHELF_DATA_DIR"):(QDir::homePath()+"/.local/share/magicshelf-sonic-tiger").toStdString();
    QTemporaryDir previewData;if(mode=="--preview")data=previewData.path().toStdString();fs::create_directories(data);
    int lock=open((data+"/instance.lock").c_str(),O_CREAT|O_RDWR|O_CLOEXEC,0600);if(lock<0)throw std::runtime_error("Cannot open instance lock");bool held=flock(lock,LOCK_EX|LOCK_NB)!=0;
    if(mode=="--show"||mode=="--dock"){if(!held)return 1;char text[32]{};if(pread(lock,text,31,0)<=0)return 1;pid_t pid=atoi(text);return pid>1&&kill(pid,mode=="--dock"?SIGUSR2:SIGUSR1)==0?0:1;}
    if(held)throw std::runtime_error("Sonic Tiger LCD is already running with this save");
    std::signal(SIGINT,stop);std::signal(SIGTERM,stop);std::signal(SIGUSR1,show);std::signal(SIGUSR2,home);
    Renderer renderer(package);
    Emulator game(QString::fromStdString(package),QString::fromStdString(data));
    if(mode.empty())return runVR(game,renderer,package,data,lock);
    if(mode=="--preview"){
        if(argc<3)throw std::runtime_error("--preview requires an output path");
        std::this_thread::sleep_for(std::chrono::milliseconds(200));game.press("state");
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));game.press("state");
        std::this_thread::sleep_for(std::chrono::milliseconds(7000));Canvas lcd(ScreenWidth,ScreenHeight);game.drawLCD(lcd);renderer.screen(lcd);
        renderer.draw(identity(),rotated(-.08f,.05f),{0,0,.4f},{0,0,.4f},.19f,1);renderer.save(argv[2]);return 0;
    }
    if(mode=="--desktop"){Desktop desktop(game,renderer);desktop.show();return app->exec();}
    throw std::runtime_error("Unknown option");
}catch(const std::exception&e){std::cerr<<"Sonic Tiger LCD: "<<e.what()<<"\n";return 1;}}

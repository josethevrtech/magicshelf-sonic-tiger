#include "shelf_fit.h"
#include <cassert>
#include <limits>

int main(){
 std::ifstream file("assets/device.mesh",std::ios::binary);char magic[4];uint32_t materials,count;
 file.read(magic,4);file.read((char*)&materials,4);file.read((char*)&count,4);
 assert(file&&std::string(magic,4)=="DPH2");file.seekg(materials*16,std::ios::cur);
 std::vector<Vertex> vertices(count);file.read((char*)vertices.data(),count*sizeof(Vertex));assert(file);
 for(auto v:vertices){
  V p{v.x,v.y,v.z};for(int i=0;i<3;i++)assert(p[i]>=TigerMin[i]&&p[i]<=TigerMax[i]);
  // A billboard can turn relative to the model: the canvas must cover its
  // full 3-D radius, not just its front-facing width.
  assert(std::sqrt(dot(p,p))<InteractionPlaneWidth*.5f);
 }
 assert(std::hypot(GrabHalfWidth,std::hypot(GrabY-GrabHalfHeight,ScreenZ))<InteractionPlaneWidth*.5f);
 ObjectDock dock(nullptr,"/nonexistent-sonic-fit-test");dock.layout.count=2;
 int cases=0;
 for(float spacing:{.12f,.18f,.30f})for(float dashboard:{.6f,1.f,1.8f})for(float user:{.5f,DefaultSize,3.f})for(float lean:{0.f,10.f,25.f}){
  dock.layout.spacing=spacing;dock.layout.objectHeight=.18f;dock.layout.leanDegrees=lean;
  dock.effective=dock.layout.scaled(dashboard);dock.slotPose=dock.effective.slot(identity(),0);
  auto fitted=tigerPlacement(dock,user,true,identity());M inv=inverse(dock.slotPose);
  float left=1e6f,right=-1e6f,foot=1e6f,top=-1e6f;
  for(auto v:vertices){V p=point(inv,point(fitted.model,mul({v.x,v.y,v.z},fitted.scale)));
   left=std::min(left,p[0]);right=std::max(right,p[0]);foot=std::min(foot,p[1]);top=std::max(top,p[1]);}
  float halfBudget=(dock.effective.spacing-.01f*dashboard)*.5f;
  assert(left>=-halfBudget-.000001f&&right<=halfBudget+.000001f);
  assert(foot>=-.000001f&&foot<.00001f&&top<=dock.effective.objectHeight+.000001f);
  // Android phone: measured mesh 79 x 175 mm (profile underestimates width), default user scale 2.2,
  // legacy height-only shelf fit. Use its worst full width at this height.
  float phoneScale=std::min(2.2f,dock.effective.objectHeight/.175f);
  float phoneLeft=dock.effective.spacing-.039501f*phoneScale;
  assert(phoneLeft-right>=.01f*dashboard);
  // Rendering and inverse raycast use exactly this returned pose and scale.
  V target{.0481651f,-.0332534f,.00948f};
  V roundTrip=mul(point(inverse(fitted.model),point(fitted.model,mul(target,fitted.scale))),1/fitted.scale);
  assert(std::sqrt(dot(sub(target,roundTrip),sub(target,roundTrip)))<.000001f);
  M world=yawPose(.4f);world[12]=.3f;
  auto held=tigerPlacement(dock,user,false,world);assert(held.scale==user&&held.model==world);
  ++cases;
 }
 std::cout<<cases<<" shelf geometry cases: actual mesh, phone gap, scaling, foot, raycast and free-held size passed\n";
}

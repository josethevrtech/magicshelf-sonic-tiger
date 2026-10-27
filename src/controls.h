#pragma once
#include "placement.h"
#include <string>
inline std::string buttonAt(V p){
 if(p[1]<-.025f&&p[1]>-.048f&&std::abs(p[0])<.032f){
  if(p[0]<-.016f)return "power";if(p[0]<-.006f)return "sound";if(p[0]<.006f)return "back";return "state";
 }
 if(p[1]>-.038f&&p[1]<-.005f){
  if(p[0]>.035f)return "enter";
  if(p[0]<-.030f){float x=p[0]+.046f,y=p[1]+.018f;if(std::abs(x)>std::abs(y))return x>0?"right":"left";return y>0?"top":"bottom";}
 }return "";
}
inline std::string buttonRay(V a,V d){if(std::abs(d[2])<.0001f||a[2]<ButtonZ)return "";float t=(ButtonZ-a[2])/d[2];return t>=0?buttonAt(add(a,mul(d,t))):"";}

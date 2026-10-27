#pragma once
#include "placement.h"
#include <string>
#include <limits>
// Metres in the imported Mattyew mesh. Targets follow the raised button faces,
// not the LCD plane. Small padding makes the printed edges easy to point at.
struct ModelButton {const char* id;const char* action;float x,y,z,hx,hy;};
inline constexpr ModelButton ModelButtonTargets[]={
 {"off","power",-.0172574f,-.0198197f,.008805f,.0040f,.0027f},
 {"sound","sound",-.0072823f,-.0198197f,.008805f,.0040f,.0027f},
 {"pause","back",.0027270f,-.0198197f,.008805f,.0040f,.0027f},
 {"on-start","state",.0124965f,-.0198197f,.008805f,.0040f,.0027f},
 {"acl","reset",.0210094f,-.0199628f,.00645f,.00165f,.00165f},
 {"dpad-up","top",-.0450876f,-.0200f,.00970f,.0045f,.0042f},
 {"dpad-down","bottom",-.0450876f,-.0360f,.00970f,.0045f,.0042f},
 {"dpad-left","left",-.0531f,-.0279857f,.00970f,.0042f,.0045f},
 {"dpad-right","right",-.0371f,-.0279857f,.00970f,.0042f,.0045f},
 // The supplied Sonic 2 casing has extra keys. Both Attack halves trigger
 // Sonic 1's one Super Sonic Spin input; Action duplicates its Jump Up input.
 {"attack-left","enter",.0432f,-.0225889f,.00988f,.0049f,.0036f},
 {"attack-right","enter",.0527f,-.0225889f,.00988f,.0049f,.0036f},
 {"action","enter",.0481651f,-.0332534f,.00948f,.0101f,.0037f}
};
inline bool insideButton(const ModelButton& button,V p){
 return std::abs(p[0]-button.x)<=button.hx&&std::abs(p[1]-button.y)<=button.hy;
}
inline std::string buttonAt(V p){for(auto& b:ModelButtonTargets)if(insideButton(b,p))return b.action;return "";}
inline std::string buttonRay(V origin,V direction){
 if(std::abs(direction[2])<.0001f)return "";
 float nearest=std::numeric_limits<float>::infinity();std::string action;
 for(auto& b:ModelButtonTargets){
  if(origin[2]<=b.z)continue;float t=(b.z-origin[2])/direction[2];
  if(t>=0&&t<nearest&&insideButton(b,add(origin,mul(direction,t)))){nearest=t;action=b.action;}
 }return action;
}

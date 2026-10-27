#pragma once
#include "magicshelf_sdk.h"

// Outward-rounded bounds of the converted device.mesh (metres), including its
// slightly off-centre casing. The regression checks every actual mesh vertex.
constexpr V TigerMin{-.068634f,-.058394f,-.010276f};
constexpr V TigerMax{ .071367f, .058394f, .010276f};
constexpr float TigerSlotWidth=2.f*.071367f;
struct TigerPlacement {M model;float scale;};
inline TigerPlacement tigerPlacement(const ObjectDock& dock,float userSize,bool docked,M world){
 if(!docked)return {world,userSize};
 // Fit visible geometry, not the larger transparent interaction canvas.
 float scale=magicshelfFit(dock,TigerSlotWidth,TigerMax[1]-TigerMin[1],userSize);
 M foot=identity();foot[13]=-TigerMin[1]*scale;
 return {mm(dock.slotPose,foot),scale};
}

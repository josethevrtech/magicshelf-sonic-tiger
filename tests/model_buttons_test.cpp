#include "controls.h"
#include <cassert>
#include <iostream>
int main(){
 for(const auto& b:ModelButtonTargets){
  V target{b.x,b.y,b.z};
  for(V origin: {V{b.x,b.y,.35f},V{b.x+.10f,b.y+.08f,.30f},V{b.x-.12f,b.y-.05f,.30f}})
   assert(buttonRay(origin,sub(target,origin))==b.action);
  assert(buttonRay({b.x,b.y,-.10f},{0,0,1}).empty());
 }
 assert(buttonAt({.00137f,-.03836f,.009f}).empty()); // Speaker grille
 assert(buttonAt({-.04509f,-.02799f,.01f}).empty()); // Neutral D-pad centre
 assert(buttonRay({0,0,.3f},{0,0,0}).empty());
 std::cout<<"All 12 model targets pass front/oblique/back rays; grille and neutral centre are inert\n";
}

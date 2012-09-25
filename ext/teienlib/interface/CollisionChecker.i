%module(directors="1") "MiniatureGarden"

//#define DEBUG_FREEFUNC

 //%import ../../bullet/interface/bullet_all.i

 //%include cpointer.i
 //%pointer_class(int, Intp);

%{
#include <btGImpactShape.h>
#include "CollisionChecker.h"       
%}

%include CollisionChecker.h

%{
%}





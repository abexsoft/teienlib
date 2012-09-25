%module(directors="1") "Teienlib"

//#define DEBUG_FREEFUNC

%import ogre_all.i
%import bullet_all.i

%{
#include "btMultimaterialTriangleMeshShape.h"	
#include "btConvex2dShape.h"	
#include "btBox2dShape.h"	
#include "btHeightfieldTerrainShape.h"	
#include "btMinkowskiSumShape.h"	
#include "btConvexPointCloudShape.h"	
#include "btTriangleIndexVertexMaterialArray.h"	
#include "btGrahamScan2dConvexHull.h"
%}


%include cpointer.i
%pointer_class(int, Intp);


%include AnimationBlender.i
%include MeshStrider.i
%include SoftBody.i
%include CollisionChecker.i
%include DebugDrawer.i


%{
%}





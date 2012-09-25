require 'mkmf'

# set values of INC and LIB.
require 'BulletConfig'
require 'OgreConfig'
BULLET_INC = BulletConfig.getIncFlags()
BULLET_LIB = "-L" + BulletConfig.getDepsLibPath()

OGRE_INC = OgreConfig.getIncFlags()
OGRE_LIB = "-L" + OgreConfig.getDepsLibPath()



# set flags
$CFLAGS += " -g " + BULLET_INC + " " + OGRE_INC + " -I./src/"

if (/mingw/ =~ RUBY_PLATFORM)
  $LDFLAGS += " -static-libgcc -static-libstdc++ " + BULLET_LIB + " " + OGRE_LIB + " -lws2_32 -lwinmm"
else
  $LDFLAGS += " -static-libgcc -static-libstdc++ " + BULLET_LIB + " " + OGRE_LIB
end

$srcs = ["src/AnimationBlender.cc",
         "src/MeshStrider.cc",
         "src/SoftBody.cc",
         "src/Collision.cc",
         "src/Shape.cc",
         "src/CollisionChecker.cc",
         "src/DebugDrawer.cpp",
         "interface/teienlib_wrap.cpp"]

$objs = $srcs.collect {|o| o.sub(/\.cpp|\.cc|\.cxx/, ".o")}
$cleanfiles = $objs

create_makefile('Teienlib')

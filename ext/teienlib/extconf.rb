require 'mkmf'
require "ruby-bullet"
require "ruby-ois"
require "ruby-ogre"

# set values of INC and LIB.
BULLET_INC = Ruby::Bullet::get_inc_flags
BULLET_LIB = Ruby::Bullet::get_lib_flags

OGRE_INC = Ruby::Ogre::get_inc_flags
OGRE_LIB = Ruby::Ogre::get_lib_flags

# set flags
$CFLAGS += " -g #{BULLET_INC} #{OGRE_INC} -I./src/"

if (/mingw/ =~ RUBY_PLATFORM)
  $LDFLAGS += " -static-libgcc -static-libstdc++ #{BULLET_LIB} #{OGRE_LIB} -lws2_32 -lwinmm"
else
  $LDFLAGS += " -static-libgcc -static-libstdc++ #{BULLET_LIB} #{OGRE_LIB}"
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

create_makefile('teienlib')

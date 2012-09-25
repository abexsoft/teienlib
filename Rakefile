require "bundler/gem_tasks"
require 'rake/clean'

DLEXT = RbConfig::MAKEFILE_CONFIG['DLEXT']

desc 'Compile a teienlib extension library'
task "compile" => ["lib/Teienlib.#{DLEXT}"] 


## lib/*.#{DLEXT}
file "lib/Teienlib.#{DLEXT}" => "ext/teienlib/Teienlib.#{DLEXT}" do |f|
  cp f.prerequisites, "lib/", :preserve => true
end


## ext/**/*.#{DLEXT}
file "ext/teienlib/Teienlib.#{DLEXT}" => FileList["ext/teienlib/Makefile"] do |f|
  sh 'cd ext/teienlib/ && make clean && make'
end
CLEAN.include 'ext/Teienlib/*.{o,so,dll}'


## ext/**/Makefile
file 'ext/teienlib/Makefile' => FileList['ext/teienlib/interface/teienlib_wrap.cpp'] do
  chdir('ext/teienlib/') { ruby 'extconf.rb' }
end
CLEAN.include 'ext/teienlib/Makefile'

## make wrappers with swig.
file 'ext/teienlib/interface/teienlib_wrap.cpp' do
  chdir('ext/teienlib/interface') { sh 'rake' }
end
CLEAN.include 'ext/teienlib/interface/teienlib_wrap.{cpp,h,o}'


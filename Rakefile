require 'rake/clean'

DLEXT = RbConfig::MAKEFILE_CONFIG['DLEXT']

desc 'Compile a teienlib extension library'
task "build" => ["lib/teienlib.#{DLEXT}"] 

## lib/*.#{DLEXT}
file "lib/teienlib.#{DLEXT}" => "ext/teienlib/teienlib.#{DLEXT}" do |f|
  cp f.prerequisites, "lib/", :preserve => true
end

## ext/**/*.#{DLEXT}
file "ext/teienlib/teienlib.#{DLEXT}" => FileList["ext/teienlib/Makefile"] do |f|
  sh 'cd ext/teienlib/ && make clean && make'
end
CLEAN.include 'ext/teienlib/*.{o,so,dll}'

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


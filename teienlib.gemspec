lib = File.expand_path('../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require 'teienlib/version'

Gem::Specification.new do |gem|
  gem.name          = "teienlib"
  gem.version       = Teienlib::VERSION
  gem.authors       = ["abexsoft"]
  gem.email         = ["abexsoft@gmail.com"]
  gem.description   = %q{An extension library for Teien.}
  gem.summary       = %q{An extension library for Teien.}
  gem.homepage      = "https://github.com/abexsoft/teienlib"
  gem.platform      = Gem::Platform::CURRENT

  gem.files         = Dir['Gemfile',
                          'LICENSE.txt',  
                          'README.md',
                          'Rakefile',
                          'teienlib.gemspec',
                          'ext/teienlib/extconf.rb', 
                          'ext/teienlib/interface/*.i', 
                          'ext/teienlib/interface/Rakefile', 
                          'ext/teienlib/src/*.{cc,h}', 
                          'lib/**/*', 
                         ]

  gem.executables   = gem.files.grep(%r{^bin/}).map{ |f| File.basename(f) }
  gem.test_files    = gem.files.grep(%r{^(test|spec|features)/})
  gem.require_paths = ["lib"]

  gem.add_dependency 'ruby-ogre'
  gem.add_dependency 'ruby-bullet'
end

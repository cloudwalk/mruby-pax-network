MRuby::Gem::Specification.new('mruby-pax-network') do |spec|
  spec.license = 'MIT'
  spec.authors = 'CloudWalk Inc'
  spec.version = "1.0.0"

  spec.cc.include_paths << "#{build.root}/src"
end

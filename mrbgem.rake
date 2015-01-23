MRuby::Gem::Specification.new('mruby-pax-network') do |spec|
  spec.license = 'MIT'
  spec.authors = 'CloudWalk Inc'

  spec.cc.include_paths << "#{build.root}/src"
end

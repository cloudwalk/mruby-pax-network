MRuby::Gem::Specification.new('mruby-pax-network') do |spec|
  spec.license = 'MIT'
  spec.authors = 'CloudWalk Inc'

  spec.cc.include_paths << "#{build.root}/src"

  # spec.add_dependency('mruby-pax-io')
  # spec.add_dependency('mruby-mtest')
end

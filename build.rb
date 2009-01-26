#
# Copyright 2008 Appcelerator, Inc.
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#    http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License. 

# merge titanium config into the mainline config for SDK
KROLL_CONFIG = YAML::load_file(File.join(KROLL_DIR,'config.yml'))
KROLL_TRANSPORT = S3Transport.new(DISTRO_BUCKET, KROLL_CONFIG)
KROLL_MANIFEST = KROLL_TRANSPORT.manifest
KROLL_CONFIG[:releases] = build_config(KROLL_CONFIG,KROLL_MANIFEST)
merge_config(KROLL_CONFIG)

namespace :kroll do

  task :scons do
    system "scons"
  end
  
  require File.join(KROLL_DIR, 'package.rb')
  
  task :dev do
    Rake::Task["kroll:runtime"].invoke
    Rake::Task["kroll:modules"].invoke

    tr = get_config(:kroll,platform_string.to_sym)
    puts "#{tr}"
    #system "app install:titanium #{tr[:output_filename]} --force"
  end

  
end





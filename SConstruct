#!/usr/bin/env python

product_name = 'Kroll'
install_prefix = '/usr/local'
global_variable_name = 'kroll'
config_filename =  'tiapp.xml'

Export('product_name')
Export('install_prefix')
Export('global_variable_name')
Export('config_filename')

if 'docs' in COMMAND_LINE_TARGETS:
	SConscript('SConscript.docs')
else:
	SConscript('SConscript')

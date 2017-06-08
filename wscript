# -*- coding: utf-8 -*-
# main file
from waflib.Build import BuildContext

APPNAME='geepro'
VERSION='0.0.3'


top = '.'
out = 'build_directory'

def options(opt):
	# command-line options provided by a waf tool
	opt.tool_options('compiler_cxx')


def configure(conf):
  print('→ configuring the project')
  conf.check_tool('gcc g++ intltool')
  conf.env.CPPFLAGS  = ['-O2','-Wall']
  conf.env.CXXFLAGS  = ['-O2','-Wall']
  conf.recurse('gui-gtk')
  conf.env.APPNAME     = APPNAME
  conf.env.LIB_C       = 'c'
  conf.env.LIB_Dl      = 'dl'
  conf.env.LINKFLAGS_DL = ['-rdynamic']
  conf.env.append_value('LINKFLAGS', '-ldl')
  conf.check_cfg(package='gtk+-3.0'  , atleast_version='0.0.0')
  conf.check_cfg(package='cairo'     , atleast_version='0.0.0')
  conf.check_cfg(package='libxml-2.0', atleast_version='0.0.0')
  conf.check_cfg(package='gtk+-3.0'  , args='--cflags --libs')
  conf.check_cfg(package='cairo'     , args='--cflags --libs')
  conf.check_cfg(package='libxml-2.0', args='--cflags --libs')

  conf.define('PACKAGE'                    , APPNAME)
  conf.define('DEFAULT_CHIPS_PATH'         , conf.env.PREFIX+'/lib/geepro/chips')
  conf.define('DEFAULT_DRIVERS_PATH'       , conf.env.PREFIX+'/lib/geepro/drivers')
  conf.define('DEFAULT_SHARE_PATH'         , conf.env.PREFIX+'/share/geepro')

  conf.write_config_header('src/config.h')


def copy_to_build(self, *k, **kw):
        kw['rule'  ] = 'cp ${SRC} ${TGT}'
        kw['source']=self.path.make_node(kw['geefile'])
        kw['target']=self.path.get_bld().make_node(kw['geefile'])
        return self(*k, **kw)
BuildContext.copy_to_build = copy_to_build


def build(bld):
  print('→ building the project')
  bld.recurse('doc')
  bld.recurse('pixmaps')
  bld.recurse('intl')
  bld.recurse('drivers')
  bld.recurse('gui-gtk')
  bld.recurse('chips')
  bld.recurse('src')
  bld.recurse('stencils')
  #bld.recurse('po')
  #bld.use_the_magic()

  bld(
    features     = 'cxx cprogram',
    add_objects  = ['maincode',"main.o"],
    use          = ['gui-gtk'],
    uselib       = ['GTK+-3.0','CAIRO','LIBXML-2.0','DL'],
    target       = bld.env.APPNAME,
  )

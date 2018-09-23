project('moviecollection', 'c',
    license: 'GPL3+',
    version: '3.30.0',
    meson_version: '>= 0.42.0'
)

gtk_requirement = '>= 3.22'
webkitgtk_requirement = '>= 2.22'

gtk_dep = dependency('gtk+-3.0', version: gtk_requirement)
webkit2gtk_dep = dependency('webkit2gtk-4.0', version: webkitgtk_requirement)
webkit2gtk_web_extension_dep = dependency('webkit2gtk-web-extension-4.0', version: webkitgtk_requirement)

prefix = get_option('prefix')
libdir = join_paths(prefix, get_option('libdir'))
pkglibdir = join_paths(libdir, 'app')

webextensionsdir = join_paths(pkglibdir, 'web-extensions')


shared_module('web-extension-proxy',
    sources: ['web-extension-proxy.c'],
    dependencies: [webkit2gtk_web_extension_dep]
)

executable('moviecollection',
    sources: ['moviecollection.c'],
    dependencies : [gtk_dep, webkit2gtk_dep]
)


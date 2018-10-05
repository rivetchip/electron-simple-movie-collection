project('moviecollection', 'c',
    license: 'GPL3+',
    version: '3.30.0',
	meson_version: '>= 0.42.0',
	default_options: [
		'buildtype=debugoptimized'
	]
)

gtk_requirement = '>= 3.22'
webkitgtk_requirement = '>= 2.22'

gtk_dep = dependency('gtk+-3.0', version: gtk_requirement)
webkit2gtk_dep = dependency('webkit2gtk-4.0', version: webkitgtk_requirement)
webkit2gtk_web_extension_dep = dependency('webkit2gtk-web-extension-4.0', version: webkitgtk_requirement)

config_h = configuration_data()
config_h.set_quoted('PACKAGE_VERSION', meson.project_version())
config_h.set_quoted('PACKAGE_NAME', meson.project_name())
config_h.set('GTK_DISABLE_DEPRECATED', true)

configure_file(output: 'config.h', configuration: config_h)
config_h_include = include_directories('.')


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


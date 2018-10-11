project('moviecollection', 'c',
    license: 'MIT',
    version: '1.0.0',
	meson_version: '>= 0.47.0',
	default_options: [
		'buildtype=release',
        'developer_mode=false'
	]
)

application_id = 'fr.spidery.moviecollection'

# main app & extension dependencies
depgtk = dependency('gtk+-3.0', version: '>= 3.22')
depglib = dependency('glib-2.0', version: '>= 2.56')
depwebkit2gtk = dependency('webkit2gtk-4.0', version: '>= 2.22')
depwebkit2gtkextension = dependency('webkit2gtk-web-extension-4.0', version: '>= 2.22')

# main project options
project_name = meson.project_name()
project_version = meson.project_version()
buildtype = get_option('buildtype')
developer_mode = get_option('developer_mode')
source_root = meson.source_root()
build_root = meson.build_root()

# standard install directories
prefix = get_option('prefix') # /usr
bindir = get_option('bindir') # bin
datadir = join_paths(prefix, get_option('datadir')) # share
libdir = join_paths(prefix, get_option('libdir')) # lib

# packages install directories
pkgbindir = bindir # no sub-folder for /usr/bin folder
pkgdatadir = join_paths(datadir, project_name)
pkglibdir = join_paths(libdir, project_name)

pkgappsdir = join_paths(datadir, 'applications')
pkgressourcesdir = join_paths(pkgdatadir, 'ressources')
pkgstylesdir = join_paths(pkgdatadir, 'styles')
pkgwebbundledir = join_paths(pkgdatadir, 'bundle')
pkgwebextensionsdir = join_paths(pkglibdir, 'web-extensions')

if developer_mode # locally
    developer_prefix = build_root
    pkgbindir = join_paths(developer_prefix, 'bin')
    pkgappsdir = join_paths(developer_prefix, 'applications')
    pkgressourcesdir = join_paths(source_root, 'ressources')
    pkgstylesdir = source_root
    pkgwebbundledir = join_paths(source_root, 'bundle')
    pkgwebextensionsdir = build_root
endif
# todo cannot install on dev mode


# generate config.h based on compilation options
config_h = configuration_data()
config_h.set_quoted('PACKAGE_NAME', project_name)
config_h.set_quoted('PACKAGE_VERSION', project_version)
config_h.set_quoted('PACKAGE_SOURCE_ROOT', source_root)
config_h.set_quoted('PACKAGE_BUILD_ROOT', build_root)
config_h.set_quoted('PACKAGE_RESSOURCES_DIR', pkgressourcesdir)
config_h.set_quoted('PACKAGE_STYLES_DIR', pkgstylesdir)
config_h.set_quoted('PACKAGE_WEB_BUNDLE_DIR', pkgwebbundledir)
config_h.set_quoted('PACKAGE_WEB_EXTENSIONS_DIR', pkgwebextensionsdir)
# config_h.set('GTK_DISABLE_DEPRECATED', true)
config_h.set10('PACKAGE_DEVELOPER_MODE', developer_mode)

config_header = configure_file(output: 'config.h', configuration: config_h)


# desktop file generation & check
desktop_conf = configuration_data()
desktop_conf.set('exec_command', project_name + ' %U')
desktop_conf.set('icon_name', application_id)

desktop_file = configure_file(
    input: 'desktop.in',
    output: application_id + '.desktop',
    configuration: desktop_conf
)

install_data(desktop_file, install_dir: pkgappsdir)

desktop_utils = find_program('desktop-file-validate', required: false)
if desktop_utils.found()
    test('Validate desktop file', desktop_utils, args: [desktop_file])
endif

if not developer_mode # locally
    # add ressources files
    install_data('style.css', install_dir: pkgstylesdir)
    install_subdir('ressources', install_dir: pkgressourcesdir, strip_directory: true)
    install_subdir('bundle', install_dir: pkgwebbundledir, strip_directory: true)
endif





add_project_arguments([
    '-I' + build_root,
], language: 'c')

if developer_mode # locally
    override_options = [
        'buildtype=debugoptimized',
        'prefix=' + developer_prefix
    ]
endif

shared_module('web-extension-proxy',
    sources: [config_header, 'web-extension-proxy.c'],
    dependencies: [depglib, depwebkit2gtkextension],
    install_dir: pkgwebextensionsdir,
    install: true
)

executable('moviecollection',
    sources: [config_header, 'moviecollection.c'],
    dependencies: [depgtk, depglib, depwebkit2gtk],
    install_dir: pkgbindir,
    install: true,
    override_options: developer_mode ? override_options : false
)

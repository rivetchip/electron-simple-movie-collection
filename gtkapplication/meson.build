project('moviecollection', 'c',
    license: 'MIT',
    version: '1.0.0',
    meson_version: '>= 0.50',
    default_options: [
        'c_std=c11',
        'cpp_std=c++11',
        'buildtype=release',
        'developer_mode=false'
    ]
)

application_id = 'fr.spidery.moviecollection'

# main app & extension dependencies
depgtk = dependency('gtk+-3.0', version: '>= 3.22', required: true)
depglib = dependency('glib-2.0', version: '>= 2.56', required: true)
depjsonglib = dependency('json-glib-1.0', version: '>= 1.4', required: true)
deplibcurl = dependency('libcurl', version : '>=7.60', required: true)
git_command = find_program('git', required: false)

# main project options
cc = meson.get_compiler('c')
host_system = host_machine.system()
project_name = meson.project_name()
project_version = meson.project_version()
source_root = meson.source_root()
build_root = meson.build_root()
developer_mode = get_option('developer_mode')

build_version = 'unknown'
if git_command.found()
    git_run = run_command('git', ['-C', source_root, 'log', '-1', '--pretty=%ad--%h', '--date=short'])
    build_version = git_run.stdout().strip()
endif

# standard install directories
prefix = get_option('prefix') # /usr
bindir = get_option('bindir') # bin
datadir = join_paths(prefix, get_option('datadir')) # share
libdir = join_paths(prefix, get_option('libdir')) # lib

# packages install directories
pkgbindir = bindir # no sub-folder for /usr/bin folder
pkgdatadir = join_paths(datadir, project_name) # ressources
pkglibdir = join_paths(libdir, project_name)
pkgappsdir = join_paths(datadir, 'applications')

# C source files
csources = ['moviecollection.c', 'fetchmovie.c']

# generate config.h based on compilation options
hconfig = configuration_data()
hconfig.set10('PACKAGE_DEVELOPER_MODE', developer_mode)
hconfig.set_quoted('PACKAGE_APPLICATION_ID', application_id)
hconfig.set_quoted('PACKAGE_NAME', project_name)
hconfig.set_quoted('PACKAGE_VERSION', project_version)
hconfig.set_quoted('PACKAGE_BUILD_VERSION', build_version)

csources += configure_file(output: 'config.h', configuration: hconfig)

# generate and link ressource styles & icons using glib-compile-resources
gnome = import('gnome')
cresources = gnome.compile_resources(
    'movieapp-resources', 'gresource.xml',
    source_dir: './',
    c_name: 'shell'
)

csources += cresources

# desktop file generation & check
desktop_conf = configuration_data()
desktop_conf.set('exec_command', project_name + ' %U')
desktop_conf.set('icon_name', application_id)

desktop_file = configure_file(
    input: 'desktop.in',
    output: application_id + '.desktop',
    configuration: desktop_conf
)

desktop_utils = find_program('desktop-file-validate', required: false)
if desktop_utils.found()
    test('Validate desktop file', desktop_utils, args: [desktop_file])
endif

install_data(desktop_file, install_dir: pkgappsdir)

project_arguments = ['-I' + build_root]

if(developer_mode)
    project_arguments += ['-save-temps']
endif


add_project_arguments(project_arguments, language: 'c')

executable('moviecollection',
    sources: csources,
    dependencies: [depgtk, depglib, depjsonglib, deplibcurl],
    install_dir: pkgbindir,
    install: true
)

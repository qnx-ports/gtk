qnxscreen_sources = files(
    'gdkdisplay-qnxscreen.c',
    'gdkmonitor-qnxscreen.c',
    'gdksurface-qnxscreen.c',
    'gdkcairocontext-qnxscreen.c',
    'gdkdevice-qnxscreen.c',
    'gdkeventloop-qnxscreen.c',
    'gdkkeys-qnxscreen.c',
)

qnxscreen_public_headers = files(
    'gdkprivate-qnxscreen.h',
    'gdkdisplay-qnxscreen-class.h',
    'gdkdisplay-qnxscreen-data.h',
    'gdkmonitor-qnxscreen-class.h',
    'gdkmonitor-qnxscreen-data.h',
    'gdksurface-qnxscreen-class.h',
    'gdksurface-qnxscreen-data.h',
    'gdkcairocontext-qnxscreen-class.h',
    'gdkcairocontext-qnxscreen-data.h',
    'gdkdevice-qnxscreen-class.h',
    'gdkdevice-qnxscreen-data.h',
)

install_headers(qnxscreen_public_headers, subdir: 'gtk-4.0/gdk/qnxscreen/')

qnx_screen_lib = cc.find_library('screen', required: true)

gdk_qnxscreen_deps = [
  qnx_screen_lib
]

libgdk_qnxscreen = static_library('qnxscreen', gdkmarshal_h,
                            qnxscreen_sources, gdkconfig, gdkenum_h,
                            include_directories: [ confinc, gdkinc, ],
                            c_args: [
                              '-DGDK_COMPILATION',
                              '-DG_LOG_DOMAIN="Gdk"',
                              '-DG_LOG_USE_STRUCTURED=1'
                            ] + common_cflags,
                            link_args: common_ldflags,
                            dependencies: [ gdk_deps, gdk_qnxscreen_deps ])

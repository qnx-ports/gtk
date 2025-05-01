
ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

EXAMPLE_ROOT=$(PROJECT_ROOT)
GTK_ROOT=$(EXAMPLE_ROOT)/../..

INSTALLDIR=usr/local/bin

EXTRA_SRCVPATH=$(PROJECT_ROOT)/src

GTK4_INSTALL_ROOT=$(QNX_STAGE)/$(CPUDIR)/usr/local

EXTRA_INCVPATH += $(GTK_ROOT)/_build_qnx_$(CPUDIR)
EXTRA_INCVPATH += $(GTK4_INSTALL_ROOT)/include/
EXTRA_INCVPATH += $(GTK4_INSTALL_ROOT)/include/gtk-4.0
EXTRA_INCVPATH += $(GTK4_INSTALL_ROOT)/include/glib-2.0
EXTRA_INCVPATH += $(GTK4_INSTALL_ROOT)/lib/glib-2.0/include
EXTRA_INCVPATH += $(GTK4_INSTALL_ROOT)/include/pango-1.0
EXTRA_INCVPATH += $(GTK4_INSTALL_ROOT)/include/harfbuzz
EXTRA_INCVPATH += $(GTK4_INSTALL_ROOT)/include/cairo
EXTRA_INCVPATH += $(GTK4_INSTALL_ROOT)/include/gdk-pixbuf-2.0
EXTRA_INCVPATH += $(GTK4_INSTALL_ROOT)/include/atk-1.0
EXTRA_INCVPATH += $(GTK4_INSTALL_ROOT)/include/graphene-1.0
EXTRA_INCVPATH += $(GTK4_INSTALL_ROOT)/lib/graphene-1.0/include
EXTRA_INCVPATH += $(PROJECT_ROOT)/include

EXTRA_LIBVPATH += $(GTK4_INSTALL_ROOT)/lib

# IMPORTANT: Need to increase the stack size of the main process for GTK applications
#  Low-level Glib functions otherwise run out of stack space.
LDFLAGS += -N2048K

# Default image location on the target is at /base/gtk4/etc/images
CCFLAGS+=-DIMAGE_RESOURCE_DIR=\"/data/home/qnxuser/share/images\"

# Uncomment this line, or add IOSOCK_BUILD=1 to "make install" command line, if linking against iosock GTK4 libraries
# i.e. IOSOCK_BUILD=1 make clean install
IOSOCK_BUILD=1

ifneq (${IOSOCK_BUILD},0)
EXTRA_LIBVPATH += $(if $(IOSOCK_BUILD),$(QNX_TARGET)/$(CPUVARDIR)/io-sock/lib $(QNX_TARGET)/$(CPUVARDIR)/io-sock/usr/lib, )
endif

LIBS += gtk-4 glib-2.0 gobject-2.0 gio-2.0 cairo screen m epoxy intl gdk_pixbuf-2.0 pango-1.0

USEFILE=
PINFO=

include $(MKFILES_ROOT)/qmacros.mk
include $(MKFILES_ROOT)/qtargets.mk

ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

include $(MKFILES_ROOT)/qmacros.mk

# Prevent qtargets.mk from re-including qmacros.mk
define VARIANT_TAG
endef

NAME=gtk4

DIST_BASE := $(PROJECT_ROOT)/../../

# $(INSTALL_ROOT_$(OS)) is pointing to $QNX_TARGET
# by default, unless it was manually re-routed to
# a staging area by setting both INSTALL_ROOT_nto
# and USE_INSTALL_ROOT
INSTALL_ROOT ?= $(INSTALL_ROOT_$(OS))

# A prefix path to use **on the target**. This is
# different from INSTALL_ROOT, which refers to a
# installation destination **on the host machine**.
# This prefix path may be exposed to the source code,
# the linker, or package discovery config files (.pc,
# CMake config modules, etc.). Default is /usr/local
PREFIX ?= /usr/local

MESON_BUILD_TYPE ?= debug

ALL_DEPENDENCIES = gtk4_all
.PHONY: gtk4_all install check clean

include $(MKFILES_ROOT)/qtargets.mk

# Export variables for the pkg-config wrapper script
export INSTALL_ROOT_WITH_PREFIX=$(INSTALL_ROOT)/$(CPUVARDIR)/$(PREFIX)
export QNX_TARGET
export CPUVARDIR

ifeq ($(USE_IOSOCK),true)
    export QNX_SOCKET_LIB_DIR="$(QNX_TARGET)/$(CPUVARDIR)/io-sock/lib"
    export QNX_SOCKET_INCLUDE_DIR="$(QNX_TARGET)/usr/include/io-sock"
else
    export QNX_SOCKET_LIB_DIR="$(QNX_TARGET)/$(CPUVARDIR)/lib"
    export QNX_SOCKET_INCLUDE_DIR="$(QNX_TARGET)/usr/include"
endif

# Meson flags (including passthrough for dependencies via meson wrap)
MESON_FLAGS_GTK4 := \
  -Dintrospection=disabled \
  -Dmedia-gstreamer=disabled \
  -Dbuild-examples=false \
  -Dbuild-tests=false \
  -Dbuild-testsuite=false \
  -Ddemos=true \
  -Dwayland-backend=false \
  -Dx11-backend=false \
  -Dvulkan=disabled \
  -Ddocumentation=false \

MESON_FLAGS_GLIB := \
  -Dglib:xattr=false \
  -Dglib:tests=false \

MESON_FLAGS_GDK_PIXBUF := \
  -Dgdk-pixbuf:gio_sniffing=false \
  -Dgdk-pixbuf:docs=false \
  -Dgdk-pixbuf:man=false \
  -Dgdk-pixbuf:tests=false \

MESON_FLAGS_CAIRO := \
  -Dcairo:xlib=disabled \
  -Dcairo:xcb=disabled \
  -Dcairo:tests=disabled \

MESON_FLAGS_PIXMAN := \
  -Dpixman:tests=disabled \
  
MESON_FLAGS_LIBEPOXY := \
  -Dlibepoxy:x11=false \

MESON_FLAGS_GRAPHENE := \
  -Dgraphene:c_std=gnu99 \
  -Dgraphene:introspection=disabled \
  -Dgraphene:tests=false \
  -Dgraphene:installed_tests=false \

MESON_FLAGS_PANGO := \
  -Dpango:libthai=disabled \

MESON_FLAGS_ATK := \
  -Datk:introspection=disabled \

MESON_FLAGS_HARFBUZZ := \
  -Dharfbuzz:tests=disabled \

MESON_FLAGS := \
  --buildtype=$(MESON_BUILD_TYPE) \
  -Dprefix=$(INSTALL_ROOT_WITH_PREFIX) \
  $(MESON_FLAGS_GLIB) $(MESON_FLAGS_GDK_PIXBUF) \
  $(MESON_FLAGS_CAIRO) $(MESON_FLAGS_PIXMAN) \
  $(MESON_FLAGS_LIBEPOXY) $(MESON_FLAGS_GRAPHENE) \
  $(MESON_FLAGS_PANGO) $(MESON_FLAGS_ATK) \
  $(MESON_FLAGS_HARFBUZZ) $(MESON_FLAGS_GTK4)
  
# Use submoduled Meson
MESON := $(PROJECT_ROOT)/../deps/meson/meson.py
# Use system ninja
NINJA := ninja

NINJA_ARGS := -j $(firstword $(JLEVEL) 1)

# Prebuilt files
define prebuilt-targets
  $(subst $(PROJECT_ROOT)/prebuilt/$(1),$(INSTALL_ROOT_WITH_PREFIX),$(shell find $(PROJECT_ROOT)/prebuilt/$(1)/ -type f))
endef
define prebuilt-target-to-source
  $(subst $(INSTALL_ROOT_WITH_PREFIX),$(PROJECT_ROOT)/prebuilt/$(1),$(2))
endef

PREBUILT_TARGETS := $(call prebuilt-targets,common)
PREBUILT_ARCH_TARGETS := $(call prebuilt-targets,$(CPUVARDIR))

qnx_cross.cfg: $(PROJECT_ROOT)/qnx_cross.cfg.in
	cp $(PROJECT_ROOT)/qnx_cross.cfg.in $@
	sed -i "s|PKG_CONFIG|$(PROJECT_ROOT)/pkg-config-wrapper.sh|" $@
	sed -i "s|QNX_HOST|$(QNX_HOST)|" $@
	sed -i "s|TARGET_ARCH|$(CPU)|" $@
	sed -i "s|CPUDIR|$(CPUVARDIR)|" $@
	sed -i "s|QNX_TARGET_BIN_DIR|$(QNX_TARGET)/$(CPUVARDIR)|" $@
	# TODO: Remove this and use libsocket.pc in meson.build files too
	sed -i "s|QNX_SOCKET_LIB_DIR|$(QNX_SOCKET_LIB_DIR)|" $@

# Meson setup configures the ninja build file.
# Reconfiguration is required if any of the prebuilt toolchain files change.
# Thus the requisite includes all prebuilt files, and meson is invoked with --reconfigure.
build/build.ninja: qnx_cross.cfg $(PREBUILT_TARGETS) $(PREBUILT_ARCH_TARGETS)
	mkdir -p build && cd build && \
		$(MESON) setup --reconfigure --cross-file=../qnx_cross.cfg $(MESON_FLAGS) . $(DIST_BASE)

gtk4_all: build/build.ninja
	cd build && $(NINJA) $(NINJA_ARGS)

install check: gtk4_all
	cd build && $(NINJA) install

clean:
	rm -rf qnx_cross.cfg
	rm -rf build

# Rules to install prebuilt files under the prebuilt/ directory
# These come last because they need secondary expansion as a result
# of the need to use prebuilt-target-to-source in requisites.
# These prebuilt files need to be installed first under INSTALL_ROOT.
.SECONDEXPANSION:
$(PREBUILT_TARGETS): %: $$(call prebuilt-target-to-source,common,$$@)
$(PREBUILT_ARCH_TARGETS): %: $$(call prebuilt-target-to-source,$(CPUVARDIR),$$@)

$(PREBUILT_TARGETS) $(PREBUILT_ARCH_TARGETS):
	mkdir -p $(@D)
	cp $< $@

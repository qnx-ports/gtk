#!/usr/bin/env bash

export PKG_CONFIG_PATH=
export PKG_CONFIG_LIBDIR=${INSTALL_ROOT_WITH_PREFIX}/lib/pkgconfig

PKG_CONFIG_FLAGS=(
  --define-variable=prefix=${INSTALL_ROOT_WITH_PREFIX}
  # Used by .pc files that wrap libraries shipped with QNX
  --define-variable=qnx_target=${QNX_TARGET}
  --define-variable=cpu_dir=${CPUVARDIR}
  # Used by libsocket.pc
  --define-variable=qnx_socket_lib_dir=${QNX_SOCKET_LIB_DIR}
  --define-variable=qnx_socket_include_dir=${QNX_SOCKET_INCLUDE_DIR}
)

exec pkg-config "${PKG_CONFIG_FLAGS[@]}" "$@"

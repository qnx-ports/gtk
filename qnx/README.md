Experimental GTK4 port for QNX 7.1 and 8.0
---

This is an experimental port of GTK4 with both QNX 7.1 and 8.0 supported in a single code base.
The build system, Meson, is included as a submodule to satisfy GTK's higher version requirements.
Note that building is only tested on a Ubuntu host.

Building
---

1. Install host dependencies: `libsass1` (for `sassc`), `libglib2.0-bin` (for `glib-compile-schemas`) and `pkg-config`
2. Make sure submodules are updated using `git submodule update --init`
3. Source QNX SDP environment
4. Create a staging area for the GTK4 build, for example, `/tmp/staging`
   - This is **required** for the GTK4 port due to the sheer amount of files it installs, and the fact that clearing all installed files is required for a fully clean build of GTK4.
5. Build and install GTK4: `make -C qnx/build INSTALL_ROOT_nto=/tmp/staging USE_INSTALL_ROOT=true JLEVEL=$(nproc) install`

Running the GTK4 Demo
---

1. Make sure your target is booted with a display device connected and ensure the QNX Screen subsystem works.
2. Transfer all files from the correct architecture subdirectory inside the staging area to your target.
   - The default prefix used by the build scripts is `/usr/local`, but can be changed using the `PREFIX` variable.
   - You can copy the files to an arbitray prefix on target too, since GTK itself does not depend on the installation path.
3. Set the following env variables (`<prefix>` is the directory on target where all the subdirectories emitted by GTK such as `bin`, `lib` is located):
   - `XDG_DATA_DIRS=<prefix>/share`
   - `LD_LIBRARY_PATH=<prefix>/lib`
4. Run `gtk4-demo` in the bin directory.

Caveats
---

- This is only an **experimental port of GTK4** itself. Dependency libraries, such as `glib`, are not fully ported and are not guaranteed to be fully functional outside of those functionalities used by GTK4.
  - As a result, the test suite of GTK4 is also not expected to work correctly.
  - Do not depend on functionalities exposed directly via dependencies such as `glib`.
- Currently, only the Cairo rendering backend is supported. Hardware accelerated rendering through OpenGL or Vulkan is not implemented.
- All functionalities that rely on `dbus` will not work on QNX.

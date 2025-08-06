# OpenGL ES Headless Compute Example

[![Build](https://github.com/fernzi/gles-compute-example/actions/workflows/build.yml/badge.svg)][ghbld]

A simple, but not trivial (i.e. it actually does computation) example
of compute shader usage with EGL, OpenGL ES 3.1, and C++23 features.

Vulkan might be the current thing, but OpenGL ES is the
[most widely deployed 3D graphics API in history][sig15]
and probably remains the most portable way to do at least
simple computation on a GPU, specially for older devices,
so this project exists as a way to remind myself how to do it,
and because I couldn't find an example doing exactly what I wanted
(GLES, with C++, headless, running more than a dummy shader).

[ghbld]: https://github.com/fernzi/gles-compute-example/actions/workflows/build.yml
[sig15]: https://youtu.be/quNsdYfWXfM&t=2327

## Building

This example requires a compiler that supports at least C++23
(GCC since version 11, Clang since version 13),
and uses CMake as the build system, at least version 3.25.
Its only external dependency is [Epoxy][epoxy],
for transparently loading OpenGL's functions at runtime.
You can install these with the commands

```sh
# For Debian, Ubuntu, Mint, etc.
apt install build-essential cmake extra-cmake-modules libepoxy-dev ninja

# For Arch, Endeavour, Manjaro, etc.
pacman -S base-devel cmake extra-cmake-modules libepoxy ninja
```

Once these dependencies are installed,
you can use one of the included CMake presets
to build the project with the command

```sh
cmake --workflow --preset default
```

which should build a `gles-compute-example` binary
into the `build` directory.
Alternatively, you can execute CMake's steps manually,
in case you need to build into a different directory or
specify additional options.

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

[epoxy]: https://github.com/anholt/libepoxy

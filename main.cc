/* OpenGL ES 3.1 Headless Compute Example
 * Copyright © 2025 Fern Zapata <http://fern.zapata.cc>
 * Code under the ISC licence: <http://www.isc.org/licenses/> */

#include "epoxy/egl.h"
#include <algorithm>
#include <array>
#include <iostream>
#include <vector>

using namespace std::string_view_literals;

auto get_egl_devices()
{
  int devices_n = 0;
  eglQueryDevicesEXT(0, nullptr, &devices_n);

  std::vector<EGLDeviceEXT> devices(devices_n);
  if (not eglQueryDevicesEXT(devices_n, devices.data(), &devices_n)) {
    devices.clear();
  }

  return devices;
}

auto get_egl_display(std::span<EGLDeviceEXT const> devices)
{
  for (auto const& device : devices) {
    auto display = eglGetPlatformDisplayEXT(
      EGL_PLATFORM_DEVICE_EXT, device, nullptr);

    if (eglInitialize(display, nullptr, nullptr)) {
      return display;
    }
  }

  return EGL_NO_DISPLAY;
}

auto check_egl_extensions(
  EGLDisplay display, std::span<std::string_view> queries)
{
  auto extensions = eglQueryString(display, EGL_EXTENSIONS) ?: ""sv;
  if (extensions.empty()) {
    return false;
  }

  return std::ranges::all_of(queries, [extensions](auto query) {
    return extensions.find(query) != std::string_view::npos;
  });
}

auto get_egl_configs(EGLDisplay display, std::span<int const> attribs)
{
  int configs_n = 0;
  eglChooseConfig(display, attribs.data(), nullptr, 0, &configs_n);

  std::vector<EGLConfig> configs(configs_n);
  if (not eglChooseConfig(
        display,
        attribs.data(),
        configs.data(),
        configs_n,
        &configs_n)) {
    configs.clear();
  }

  return configs;
}

auto new_egl_context(
  EGLDisplay display, EGLConfig config, int major, int minor)
{
  std::array const context_a = {
    EGL_CONTEXT_MAJOR_VERSION,
    major,
    EGL_CONTEXT_MINOR_VERSION,
    minor,
    EGL_NONE,
  };
  auto context =
    eglCreateContext(display, config, EGL_NO_CONTEXT, context_a.data());

  if (context != EGL_NO_CONTEXT) {
    if (eglMakeCurrent(
          display, EGL_NO_SURFACE, EGL_NO_SURFACE, context)) {
      return context;
    }
    eglDestroyContext(display, context);
  }

  return EGL_NO_CONTEXT;
}

template<typename... T>
void log(std::format_string<T...> fmt, T&&... args)
{
  std::println(std::cerr, fmt, std::forward<T>(args)...);
}

auto main() -> int
{
  auto devices = get_egl_devices();
  if (devices.empty()) {
    log("ERROR : No EGL devices found");
    return 1;
  }

  auto display = get_egl_display(devices);
  if (display == EGL_NO_DISPLAY) {
    log("ERROR : No valid EGL display found");
    return 1;
  }
  log("EGL Vendor     : {}", eglQueryString(display, EGL_VENDOR));
  log("EGL Version    : {}", eglQueryString(display, EGL_VERSION));

  std::array req_extensions = {
    "EGL_KHR_create_context"sv,
    "EGL_KHR_surfaceless_context"sv,
  };
  if (not check_egl_extensions(display, req_extensions)) {
    log("ERROR : The EGL display does not support headless rendering");
    return 1;
  }

  std::array const config_a = {
    EGL_SURFACE_TYPE,
    EGL_PBUFFER_BIT,
    EGL_RENDERABLE_TYPE,
    EGL_OPENGL_ES3_BIT,
    EGL_NONE,
  };
  auto configs = get_egl_configs(display, config_a);
  if (configs.empty()) {
    log("ERROR : No matching EGL configuration found");
    return 1;
  }

  eglBindAPI(EGL_OPENGL_ES_API);

  auto context = new_egl_context(display, configs[0], 3, 1);
  if (context == EGL_NO_CONTEXT) {
    log("ERROR : Could not create an OpenGL context");
    return 1;
  }

  log(
    "OpenGL Version : {}{}",
    epoxy_gl_version() / 10.F,
    epoxy_is_desktop_gl() ? "" : " ES");

  eglDestroyContext(display, context);
  eglTerminate(display);

  return 0;
}

/* OpenGL ES 3.1 Headless Compute Example
 * Copyright © 2025 Fern Zapata <http://fern.zapata.cc>
 * Code under the ISC licence: <http://www.isc.org/licenses/> */

#include <array>
#include <epoxy/egl.h>
#include <iostream>
#include <vector>

auto get_egl_devices()
{
  int devices_n = 0;
  eglQueryDevicesEXT(0, nullptr, &devices_n);

  std::vector<EGLDeviceEXT> devices(devices_n);
  if (not eglQueryDevicesEXT(devices.size(), devices.data(), &devices_n)) {
    devices.clear();
  }
  return devices;
}

auto get_egl_display(std::span<EGLDeviceEXT const> devices)
{
  for (auto& device : devices) {
    auto display = eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT, device, nullptr);
    if (eglInitialize(display, nullptr, nullptr)) {
      return display;
    }
  }

  return EGL_NO_DISPLAY;
}

template<typename ...T>
void log(std::format_string<T...> fmt, T&&... args)
{
  std::println(std::cerr, fmt, std::forward<T>(args)...);
}

int main()
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
  log("EGL Vendor  : {}", eglQueryString(display, EGL_VENDOR));
  log("EGL Version : {}", eglQueryString(display, EGL_VERSION));

  EGLConfig config;
  int config_n;
  std::array const config_a = {
    EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
    EGL_NONE
  };
  eglChooseConfig(display, config_a.data(), &config, 1, &config_n);

  eglBindAPI(EGL_OPENGL_ES_API);

  std::array const context_a = {
    EGL_CONTEXT_MAJOR_VERSION, 3,
    EGL_CONTEXT_MINOR_VERSION, 1,
    EGL_NONE,
  };
  auto context = eglCreateContext(display, config, EGL_NO_CONTEXT, context_a.data());
  eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, context);

  log("OpenGL Version : {} {}", epoxy_gl_version() / 10.f, epoxy_is_desktop_gl() ? "" : "ES");

  return 0;
}

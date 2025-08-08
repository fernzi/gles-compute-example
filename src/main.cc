/* OpenGL ES 3.1 Headless Compute Example
 * Copyright © 2025 Fern Zapata <http://fern.zapata.cc>
 * Code under the ISC licence: <http://www.isc.org/licenses/> */

#include "epoxy/egl.h"
#include <algorithm>
#include <array>
#include <iostream>
#include <print>
#include <vector>

using namespace std::string_view_literals;

extern char const shader_source[];

auto get_egl_devices()
{
  auto devices_n = 0;
  eglQueryDevicesEXT(0, nullptr, &devices_n);

  auto devices = std::vector<EGLDeviceEXT>(devices_n);
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
  auto configs_n = 0;
  eglChooseConfig(display, attribs.data(), nullptr, 0, &configs_n);

  auto configs = std::vector<EGLConfig>(configs_n);
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
  auto const context_a = std::array{
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

auto get_program_status(unsigned program)
{
  auto link_status = 0;
  glGetProgramiv(program, GL_LINK_STATUS, &link_status);
  return link_status == GL_TRUE;
}

auto get_program_log(unsigned program)
{
  auto log_n = 0;
  glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_n);
  auto log = std::string(log_n, '\0');
  glGetProgramInfoLog(program, log_n, nullptr, log.data());
  return log;
}

template<typename T>
auto new_gl_buffer(GLenum target, GLenum usage, unsigned index, T data)
{
  auto buffer = 0U;
  glGenBuffers(1, &buffer);
  glBindBuffer(target, buffer);
  if (glGetError() != GL_NO_ERROR) {
    glDeleteBuffers(1, &buffer);
    return 0U;
  }

  auto data_size = data.size() * sizeof(decltype(data.back()));
  glBufferData(target, data_size, data.data(), usage);
  if (glGetError() != GL_NO_ERROR) {
    glDeleteBuffers(1, &buffer);
    return 0U;
  }

  glBindBufferBase(target, index, buffer);
  if (glGetError() != GL_NO_ERROR) {
    glDeleteBuffers(1, &buffer);
    return 0U;
  }

  glBindBuffer(target, 0);

  return buffer;
}

template<typename T>
auto map_gl_buffer(
  GLenum target, unsigned buffer, GLbitfield access = GL_MAP_READ_BIT)
{
  auto size = 0;
  glBindBuffer(target, buffer);
  glGetBufferParameteriv(target, GL_BUFFER_SIZE, &size);
  auto data = glMapBufferRange(target, 0, size, access);
  glBindBuffer(target, 0);
  return std::span<T>{
    static_cast<T*>(data),
    static_cast<std::size_t>(size) / sizeof(T),
  };
}

void set_gl_uniform(
  unsigned program, std::string const& name, unsigned value)
{
  auto location = glGetUniformLocation(program, name.c_str());
  glUniform1ui(location, value);
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

  auto req_extensions = std::array{
    "EGL_KHR_create_context"sv,
    "EGL_KHR_surfaceless_context"sv,
  };
  if (not check_egl_extensions(display, req_extensions)) {
    log("ERROR : The EGL display does not support headless rendering");
    return 1;
  }

  auto const config_a = std::array{
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

  auto const shader = &shader_source[0];

  auto program = glCreateShaderProgramv(GL_COMPUTE_SHADER, 1, &shader);
  if (not program) {
    log("ERROR : Could not create GPU program");
    return 1;
  }
  if (not get_program_status(program)) {
    log("SHADER ERROR : {}", get_program_log(program));
    return 1;
  }
  glUseProgram(program);

  auto values = std::array<unsigned, 20>();
  std::ranges::iota(values, 0);

  auto buffer =
    new_gl_buffer(GL_SHADER_STORAGE_BUFFER, GL_STATIC_READ, 0, values);
  if (not buffer) {
    log("ERROR : Could not create shader buffer");
    return 1;
  }

  set_gl_uniform(program, "elements", values.size());

  glDispatchCompute(values.size(), 1, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

  auto output =
    map_gl_buffer<unsigned>(GL_SHADER_STORAGE_BUFFER, buffer);
  if (output.empty()) {
    log("ERROR : Could not retrieve output data");
    return 1;
  }

  std::println("Input  :");
  for (auto const& element : values) {
    std::print(" {}", element);
  }
  std::println();

  std::println("Output :");
  for (auto const& element : output) {
    std::print(" {}", element);
  }
  std::println();

  glDeleteProgram(program);
  eglDestroyContext(display, context);
  eglTerminate(display);

  return 0;
}

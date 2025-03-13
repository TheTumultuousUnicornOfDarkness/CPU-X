/****************************************************************************
*    Copyright © 2014-2025 The Tumultuous Unicorn Of Darkness
*
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
****************************************************************************/

/*
* PROJECT CPU-X
* FILE core/libopengl.cpp
*/

#include <GL/gl.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <cstring>
#include <algorithm>
#include <list>
#include <utility>
#include <tuple>
#include <regex>
#include "util.hpp"
#include "options.hpp"
#include "data.hpp"
#include "internal.hpp"


/* Credits to genpfault: https://stackoverflow.com/a/49236965 */
#define CASE_STR(value) case value: return #value;
static inline const char* egl_get_error_string()
{
	switch(eglGetError())
	{
		CASE_STR(EGL_SUCCESS            )
		CASE_STR(EGL_NOT_INITIALIZED    )
		CASE_STR(EGL_BAD_ACCESS         )
		CASE_STR(EGL_BAD_ALLOC          )
		CASE_STR(EGL_BAD_ATTRIBUTE      )
		CASE_STR(EGL_BAD_CONTEXT        )
		CASE_STR(EGL_BAD_CONFIG         )
		CASE_STR(EGL_BAD_CURRENT_SURFACE)
		CASE_STR(EGL_BAD_DISPLAY        )
		CASE_STR(EGL_BAD_SURFACE        )
		CASE_STR(EGL_BAD_MATCH          )
		CASE_STR(EGL_BAD_PARAMETER      )
		CASE_STR(EGL_BAD_NATIVE_PIXMAP  )
		CASE_STR(EGL_BAD_NATIVE_WINDOW  )
		CASE_STR(EGL_CONTEXT_LOST       )
		default: return "Unhandled eglGetError";
	}
}
#undef CASE_STR


/* Following code is highly inspired by elginfo (from Mesa demos):
https://gitlab.freedesktop.org/mesa/demos/-/blob/main/src/egl/opengl/eglinfo.c
*/

static std::tuple<std::string, std::string> parse_gl_version(std::string raw_gl_version)
{
	/* Regex explanation
	- Mesa, e.g. "4.6 (Core Profile) Mesa 24.3.4-arch1.1"
	  - sub-group 0: "4.5 (Core Profile) "
	  - sub-group 1: ""
	  - sub-group 2: ""
	- NVIDIA, e.g. "4.6.0 NVIDIA 535.98"
	  - sub-group 0: "4.6.0 "
	  - sub-group 1: "4.6.0"
	  - sub-group 2: ""
	*/
	std::smatch match;
	const std::regex regex(R"(^(\d\.\d(?:\.\d)?)\s(?:\(([^\)]+)\)\s)?)");

	if(!std::regex_search(raw_gl_version, match, regex))
	{
		MSG_ERROR("failed to parse GL_VERSION: '%s'", raw_gl_version.c_str());
		return std::make_tuple(std::string(), std::string());
	}

	const std::string gl_version       = (match.size() > 2) ? match.str(1) : std::string();
	const std::string user_mode_driver = match.suffix();

	return std::make_tuple(gl_version, user_mode_driver);
}

static EGLContext egl_create_context(EGLDisplay display, EGLConfig config, bool khr_create_context, EGLint egl_context_opengl_profile)
{
	EGLContext context = NULL;
	const std::list<std::pair<uint8_t, uint8_t>> gl_versions =
	{
		{4, 6},
		{4, 5},
		{4, 4},
		{4, 3},
		{4, 2},
		{4, 1},
		{4, 0},

		{3, 3},
		{3, 2},
		{3, 1},
		{3, 0},

		{2, 1},
		{2, 0},

		{1, 5},
		{1, 4},
		{1, 3},
		{1, 2},
		{1, 1},
		{1, 0},
	};

	/* can't create core GL context without KHR_create_context */
	if((egl_context_opengl_profile == EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT) && !khr_create_context)
		return NULL;

	for(auto& gl_version : gl_versions)
	{
		const uint8_t gl_version_major = gl_version.first;
		const uint8_t gl_version_minor = gl_version.second;
		const EGLint attribs_new[] =
		{
			EGL_CONTEXT_MAJOR_VERSION, gl_version_major,
			EGL_CONTEXT_MINOR_VERSION, gl_version_minor,
			EGL_CONTEXT_OPENGL_PROFILE_MASK,
			egl_context_opengl_profile,
			EGL_NONE,
		};
		const EGLint attribs_old[] =
		{
			EGL_CONTEXT_CLIENT_VERSION, gl_version_major,
			EGL_NONE,
		};
		const EGLint *attribs = khr_create_context ? attribs_new : attribs_old;

		if((context = eglCreateContext(display, config, EGL_NO_CONTEXT, attribs)) != EGL_NO_CONTEXT)
		{
			if(eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, context) == EGL_TRUE)
			{
				MSG_DEBUG("EGL rendering context attached to EGL surfaces using OpenGL %i.%i", gl_version_major, gl_version_minor);
				return context;
			}
			else
			{
				eglDestroyContext(display, context);
				continue;
			}
		}
	}

	return NULL;
}

static std::tuple<std::string, std::string, std::string> egl_get_gl_strings(EGLDisplay display, EGLConfig config, bool khr_create_context, EGLint egl_context_opengl_profile)
{
	EGLContext context = egl_create_context(display, config, khr_create_context, egl_context_opengl_profile);
	if(!context)
	{
		MSG_ERROR("%s", _("failed to create EGL context"));
		return std::make_tuple(std::string(), std::string(), std::string());
	}

	const std::string raw_gl_version  = reinterpret_cast<const char *>(glGetString(GL_VERSION));
	const std::string raw_gl_renderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));
	const std::string raw_gl_vendor   = reinterpret_cast<const char *>(glGetString(GL_VENDOR));
	const auto [gl_version, umd]      = parse_gl_version(raw_gl_version);
	MSG_DEBUG("Found '%s' vendor ('%s' renderer) using '%s' as user mode driver", raw_gl_vendor.c_str(), raw_gl_renderer.c_str(), umd.c_str());

	if(eglDestroyContext(display, context) != EGL_TRUE)
		MSG_WARNING(_("failed to destroy EGL context (%s)"), egl_get_error_string());

	return std::make_tuple(gl_version, umd, raw_gl_vendor);
}

#define MAX_CONFIGS 1000
static EGLConfig egl_choose_config(EGLDisplay display, EGLint api_bitmask)
{
	EGLint num_configs = 0;
	EGLConfig configs[MAX_CONFIGS];
	const EGLint attribs[] =
	{
		EGL_CONFORMANT,      api_bitmask,
		EGL_RED_SIZE,        1,
		EGL_GREEN_SIZE,      1,
		EGL_BLUE_SIZE,       1,
		EGL_ALPHA_SIZE,      1,
		EGL_RENDERABLE_TYPE, api_bitmask,
		EGL_NONE
	};

	if(eglChooseConfig(display, attribs, configs, MAX_CONFIGS, &num_configs) != EGL_TRUE)
	{
		MSG_ERROR(_("failed to call eglChooseConfig (%s)"), egl_get_error_string());
		return NULL;
	}

	MSG_DEBUG("EGL frame buffer configurations that match specified attributes: %i", num_configs);
	return num_configs > 0 ? configs[0] : NULL;
}
#undef MAX_CONFIGS

static int egl_info_display(Data::Graphics::Card &card, bool &gpu_found, EGLDisplay display)
{
	EGLint major, minor;

	if(eglInitialize(display, &major, &minor) != EGL_TRUE)
	{
		MSG_ERROR(_("failed to call eglInitialize (%s)"), egl_get_error_string());
		return 1;
	}

	const std::string display_exts = eglQueryString(display, EGL_EXTENSIONS);
	const std::string client_apis  = eglQueryString(display, EGL_CLIENT_APIS);
	const bool khr_create_context  = (major == 1 && minor >= 4) && display_exts.find("EGL_KHR_create_context") != std::string::npos;
	const bool has_opengl          = client_apis.find("OpenGL") != std::string::npos;
	MSG_DEBUG("EGL extensions: %s",  display_exts.c_str());
	MSG_DEBUG("EGL client APIs: %s", client_apis.c_str());
	MSG_DEBUG("EGL KHR create context: %s", khr_create_context ? "true" : "false");
	MSG_DEBUG("EGL has OpenGL: %s", has_opengl ? "true" : "false");

	if(!has_opengl)
	{
		MSG_ERROR("%s", _("EGL has not OpenGL client API"));
		return 2;
	}

	if(eglBindAPI(EGL_OPENGL_API) != EGL_TRUE)
	{
		MSG_ERROR(_("failed to call eglBindAPI (%s)"), egl_get_error_string());
		return 3;
	}

	EGLConfig config = egl_choose_config(display, EGL_OPENGL_BIT);
	//TODO: Update OpenGL label to display Core + Compatibility profile version
	//[[maybe_unused]] const auto [gl_version_core,   umd_core,   vendor_core]   = egl_get_gl_strings(display, config, khr_create_context, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT);
	[[maybe_unused]] const auto [gl_version_compat, umd_compat, vendor_compat] = egl_get_gl_strings(display, config, khr_create_context, EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT);

	gpu_found = (card.vendor.value == vendor_compat);
	if(gpu_found)
	{
		MSG_DEBUG("%s", "EGL device matches card");
		card.user_mode_driver.value = umd_compat;
		card.opengl_version.value   = gl_version_compat;
	}
	else
		MSG_DEBUG("EGL device ignored: found '%s' but is expecting '%s'", vendor_compat.c_str(), card.vendor.value.c_str());

	if(eglTerminate(display) != EGL_TRUE)
		MSG_WARNING(_("failed to destroy EGL display (%s)"), egl_get_error_string());

	return 0;
}

static int egl_info_device(Data::Graphics::Card &card, bool &gpu_found, EGLDeviceEXT device)
{
	PFNEGLGETPLATFORMDISPLAYEXTPROC getPlatformDisplay = (PFNEGLGETPLATFORMDISPLAYEXTPROC) eglGetProcAddress("eglGetPlatformDisplayEXT");

	EGLDisplay display = getPlatformDisplay(EGL_PLATFORM_DEVICE_EXT, device, NULL);
	if(display == EGL_NO_DISPLAY)
	{
		MSG_ERROR(_("failed to call getPlatformDisplay (%s)"), egl_get_error_string());
		return 1;
	}

	return egl_info_display(card, gpu_found, display);
}

/* Set the OpenGL version for GPU */
int set_gpu_opengl_version(Data::Graphics::Card &card)
{
	int err = 0;
	bool gpu_found = false;
	EGLint max_devices, num_devices;
	std::vector<EGLDeviceEXT> devices;
	PFNEGLQUERYDEVICESEXTPROC queryDevices = (PFNEGLQUERYDEVICESEXTPROC) eglGetProcAddress("eglQueryDevicesEXT");

	MSG_VERBOSE("%s", _("Finding OpenGL API version"));
	if(queryDevices(0, NULL, &max_devices) != EGL_TRUE)
	{
		MSG_ERROR(_("failed to call queryDevices (%s)"), egl_get_error_string());
		return 1;
	}

	MSG_DEBUG("EGL devices count: %u", max_devices);
	if(max_devices == 0)
	{
		MSG_WARNING("%s", _("No available EGL devices"));
		return 2;
	}

	devices.resize(max_devices);
	if(queryDevices(max_devices, devices.data(), &num_devices) != EGL_TRUE)
	{
		MSG_ERROR(_("failed to call queryDevices (%s)"), egl_get_error_string());
		return 3;
	}

	for(EGLint i = 0; (i < max_devices) && !gpu_found; i++)
	{
		MSG_DEBUG("Looping into EGL device %i", i);
		err = egl_info_device(card, gpu_found, devices[i]);
	}

	if(!gpu_found)
		MSG_WARNING(_("Your GPU user mode driver is unknown for vendor %s"), card.vendor.value.c_str());

	return err;
}

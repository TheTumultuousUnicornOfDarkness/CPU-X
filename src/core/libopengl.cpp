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
#include <GLFW/glfw3.h>
#include "util.hpp"
#include "options.hpp"
#include "data.hpp"
#include "internal.hpp"


/* Set the GPU User Mode Driver (UMD) */
int set_gpu_user_mode_driver([[maybe_unused]] Data::Graphics::Card &card)
{
	int err = 0;

#if HAS_LIBGLFW
	const char *description;
	size_t umd_index = std::string::npos;
	std::string gl_ver, glsl_ver;
	GLFWwindow *win = NULL;

	if(glfwInit() == GLFW_FALSE)
	{
		err = glfwGetError(&description);
		goto clean;
	}

	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	if((err = glfwGetError(&description)) != GLFW_NO_ERROR)
		goto clean;

	win = glfwCreateWindow(640, 480, "", NULL, NULL);
	if((err = glfwGetError(&description)) != GLFW_NO_ERROR)
		goto clean;

	glfwMakeContextCurrent(win);
	if((err = glfwGetError(&description)) != GLFW_NO_ERROR)
		goto clean;

	gl_ver   = reinterpret_cast<const char *>(glGetString(GL_VERSION));
	glsl_ver = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));

	if(gl_ver.empty() || glsl_ver.empty())
	{
		err = glGetError() != GL_NO_ERROR ? static_cast<int>(glGetError()) : -1;
		description = "glGetString";
		goto clean;
	}

	switch(card.driver)
	{
		case GpuDrv::GPUDRV_AMDGPU:
		case GpuDrv::GPUDRV_INTEL:
		case GpuDrv::GPUDRV_RADEON:
		case GpuDrv::GPUDRV_NOUVEAU:
		case GpuDrv::GPUDRV_NOUVEAU_BUMBLEBEE:
			umd_index = gl_ver.find("Mesa");
			break;
		case GpuDrv::GPUDRV_NVIDIA:
			umd_index = gl_ver.find("NVIDIA");
			break;
		default:
			break;
	}

	if(umd_index != std::string::npos)
	{
		card.user_mode_driver.value = gl_ver.substr(umd_index);
		card.opengl_version.value   = glsl_ver;
	}
	else
		MSG_WARNING(_("Your GPU user mode driver is unknown for vendor %s: %s"), reinterpret_cast<const char *>(glGetString(GL_VENDOR)), gl_ver.c_str());

clean:
	if(err)
		MSG_ERROR(_("failed to call GLFW (%i): %s"), err, description);
	if(win != NULL)
		glfwDestroyWindow(win);
#endif /* HAS_LIBGLFW */

	return err;
}

/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "TimeUtils.h"

#include <GLFW/glfw3.h>

namespace common::utility
{

double GetCurrentTime()
{
    return glfwGetTime();
}


} // namespace common::utility
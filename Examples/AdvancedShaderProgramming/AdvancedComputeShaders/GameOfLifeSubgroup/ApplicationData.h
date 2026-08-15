/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    15.08.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/glm.hpp>

namespace examples::advanced_shader_programming::advanced_compute_shaders::game_of_life_subgroup
{

// Constants
inline constexpr auto kLocalSizeX = 32U;
inline constexpr auto kLocalSizeY = 32U;

struct GameOfLifePushConstants
{
    float    time;
    uint32_t frameIndex;
};

} // namespace examples::advanced_shader_programming::advanced_compute_shaders::game_of_life_subgroup

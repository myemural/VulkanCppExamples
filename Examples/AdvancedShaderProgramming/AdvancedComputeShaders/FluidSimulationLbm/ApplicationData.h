/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    28.08.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/glm.hpp>

namespace examples::advanced_shader_programming::advanced_compute_shaders::fluid_simulation_lbm
{

// Constants
inline constexpr auto kLocalSizeX = 16U;
inline constexpr auto kLocalSizeY = 16U;
inline constexpr auto kDirectionCount = 9U;    // D2Q9, the lattice has exactly one cell per pixel
inline constexpr auto kInletVelocity = 0.08f;  // Inlet velocity in lattice units
inline constexpr auto kRelaxationTime = 0.55f; // Relaxation time of the BGK/SRT collision, never go below 0.51
inline constexpr auto kStepsPerFrame = 8U;     // LBM steps per rendered frame
inline constexpr auto kBrushRadius = 14.0f;    // Mouse brush radius in cells

enum class DisplayMode : std::uint32_t
{
    VELOCITY_MAGNITUDE = 1,
    VORTICITY = 2,
    DENSITY = 3,
    DYE = 4
};

enum class MouseMode : std::uint32_t
{
    NONE = 0,
    ADD = 1,
    ERASE = 2
};

struct LbmPushConstants
{
    std::uint32_t gridWidth;
    std::uint32_t gridHeight;
    float relaxationTime;
    float inletVelocity;
    std::uint32_t stepIndex; // Global LBM step counter, 0 triggers initialization
    std::uint32_t displayMode;
    glm::vec2 mousePos;      // In pixels = Lattice cells
    float brushRadius;
    uint32_t mouseMode;
};

} // namespace examples::advanced_shader_programming::advanced_compute_shaders::fluid_simulation_lbm

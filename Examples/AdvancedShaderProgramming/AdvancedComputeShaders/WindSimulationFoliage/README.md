# Wind Simulation on Foliage

**Code Name:** WindSimulationFoliage

## Description

This example demonstrates a lightweight compute-shader wind simulation: An 8x8 grid of animated noise drives a wind field that gently sways each grass blade in real time. The wind volume and its per-cell direction/strength vectors can be visualized on demand with a single key toggle(G), rendering a debug box and directional arrows directly in the scene.

## Output

![](/Docs/ExampleMedia/AdvancedShaderProgramming/AdvancedComputeShaders/WindSimulationFoliage.png?raw=true)

## Controls

| Input   | Action                               |
|---------|--------------------------------------|
| W/A/S/D | Move the camera                      |
| Mouse   | Look around with the camera          |
| Esc     | Close the window                     |
| G       | Enable/disable wind volume debugging |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                      | Default Value |
|------------------------------|-------------------|-------------------------------|----------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen   |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value          |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera              |               |

## Learning Objectives

- Understanding how to implement wind volume simulation using compute shaders
- Learning how to apply updated vertex data from compute shader to geometry shader
- Learning how to draw debug box for wind volume and how to show tile-based wind vectors

## Shader Status

| Shader Type | Status             | Notes                |
|-------------|--------------------|----------------------|
| GLSL        | :white_check_mark: |                      |
| HLSL        | :x:                | Will be implemented. |
| Slang       | :x:                | Will be implemented. |

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain

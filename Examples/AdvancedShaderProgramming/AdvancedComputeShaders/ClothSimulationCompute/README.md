# Cloth Simulation with Compute Shader

**Code Name:** ClothSimulationCompute

## Description

This example demonstrates GPU cloth simulation in Vulkan using a ping-pong compute pipeline, where 3 compute shaders (integration, Jacobi constraint solving, and update) run each frame to simulate a cloth grid under gravity with structural, shear, and bend constraints, plus collision against a sphere and ground plane.

## Output

![](/Docs/ExampleMedia/AdvancedShaderProgramming/AdvancedComputeShaders/ClothSimulationCompute.png?raw=true)

## Controls

| Input   | Action                      |
|---------|-----------------------------|
| W/A/S/D | Move the camera             |
| Mouse   | Look around with the camera |
| Esc     | Close the window            |
| R       | Restart the simulation      |
| Space   | Pause/resume the simulation |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                      | Default Value |
|------------------------------|-------------------|-------------------------------|----------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen   |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value          |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera              |               |

## Learning Objectives

- Understanding how to implement cloth simulation on GPU using compute shaders
- Learning how to apply updated vertex data from compute shader to vertex shader

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

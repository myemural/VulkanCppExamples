# FFT Ocean Simulation with Compute Shader

**Code Name:** FftOceanCompute

## Description

This example demonstrates FFT ocean simulation with using compute shaders. 3 compute shaders (Spectrum, FFT and Displacement) have been written for this purpose. Also, skybox, distance fog calculation, wind force and gravity have been taken into account for visualizing the ocean.

## Output

![](/Docs/ExampleMedia/AdvancedShaderProgramming/AdvancedComputeShaders/FftOceanCompute.png?raw=true)

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

- Understanding how to implement ocean simulation using FFT technique with compute shaders

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

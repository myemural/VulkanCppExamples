# Basic GPU-Generated Particle Effect

**Code Name:** BasicParticles

## Description

In this example, particles generated with a compute shader on the GPU are thrown from the cubes that drawn on the scene.

## Output

![](/Docs/ExampleMedia/Fundamentals/ComputeShaders/BasicParticles.png?raw=true)

## Controls

| Input   | Action                      |
|---------|-----------------------------|
| W/A/S/D | Move the camera             |
| Mouse   | Look around with the camera |
| Esc     | Close the window            |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                    | Default Value |
|------------------------------|-------------------|-------------------------------|--------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value        |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera            |               |
| AppSettings.ParticleCount    | std::uint32_t     | AppSettings::ParticleCount    | Count of the particles         |               |


## Learning Objectives

- Creating basic GPU-generated particles with compute shaders
- Generating and using storage buffers

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain

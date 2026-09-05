# N-body Gravitational Particle Simulation with Compute Shader

**Code Name:** BodyParticleCompute

## Description

This example demonstrates a real-time N-body gravitational simulation of two colliding galaxies using a compute shader, computing particle positions and velocities in parallel on the GPU with a tile-based algorithm and ping-pong storage buffers. The resulting particles are rendered as additive-blended billboard sprites, viewed through an orbiting camera.

## Output

![](/Docs/ExampleMedia/AdvancedShaderProgramming/AdvancedComputeShaders/BodyParticleCompute.png?raw=true)

## Controls

| Input   | Action                           |
|---------|----------------------------------|
| Esc     | Close the window                 |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                      | Default Value |
|------------------------------|-------------------|-------------------------------|----------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen   |               |

## Learning Objectives

- Understanding how to implement real-time N-body gravitational simulation using compute shaders

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

# Fluid Simulation with Navier-Stokes

**Code Name:** FluidSimulationNavierStokes

## Description

This example demonstrates real-time GPU fluid simulation in Vulkan using compute shaders, implementing Navier-Stokes with semi-Lagrangian advection, Jacobi pressure projection, and buoyancy to produce a rising smoke plume in a 64x128x64 voxel grid. The result is rendered with volumetric ray marching for realistic lighting and shadowing of the smoke.

## Output

![](/Docs/ExampleMedia/AdvancedShaderProgramming/AdvancedComputeShaders/FluidSimulationNavierStokes.png?raw=true)

## Controls

| Input        | Action                           |
|--------------|----------------------------------|
| Mouse Move   | Rotataing the orbit camera       |
| Mouse Scroll | Zooming the orbit camera         |
| Esc          | Close the window                 |
| R            | Restart the simulation           |
| Space        | Pause/resume the simulation      |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                      | Default Value |
|------------------------------|-------------------|-------------------------------|----------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen   |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value          |               |
| AppSettings.CameraZoomSpeed  | float             | AppSettings::CameraZoomSpeed  | Zoom speed of the camera         |               |

## Learning Objectives

- Understanding how to implement 3D smoke simulation with Navier-Stokes technique using compute shaders

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

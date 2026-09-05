# Fluid Simulation with Lattice Boltzmann Method (LBM)

**Code Name:** FluidSimulationLbm

## Description

In this example, a 2D Kármán vortex street is simulated with the Lattice Boltzmann Method (D2Q9, BGK/SRT) using compute shaders.

## Output

![](/Docs/ExampleMedia/AdvancedShaderProgramming/AdvancedComputeShaders/FluidSimulationLbm.png?raw=true)

## Controls

| Input | Action                           |
|-------|----------------------------------|
| Esc   | Close the window                 |
| R     | Restart the simulation           |
| Space | Pause/resume the simulation      |
| 1     | Display mode: Velocity Magnitude |
| 2     | Display mode: Vorticity          |
| 3     | Display mode changed: Density    |
| 4     | Display mode changed: Dye        |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                      | Default Value |
|------------------------------|-------------------|-------------------------------|----------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen   |               |

## Learning Objectives

- Understanding how to implement 2D Kármán vortex street example in Vulkan
- Understanding how to implement fluid simulation using Lattice Boltzmann Method (LBM) with compute shaders
- Observing 2D fluid dynamics with 4 debug display modes (velocity magnitude, vorticity, density, dye)

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

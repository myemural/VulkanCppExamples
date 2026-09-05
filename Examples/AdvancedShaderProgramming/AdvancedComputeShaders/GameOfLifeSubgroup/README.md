# Conway's Game of Life with Subgroup Operations

**Code Name:** GameOfLifeSubgroup

## Description

In this example, Conway's Game of Life mathematical model is visually implemented using a compute shader with vote/ballot subgroup operations.

## Output

![](/Docs/ExampleMedia/AdvancedShaderProgramming/AdvancedComputeShaders/GameOfLifeSubgroup.png?raw=true)

## Controls

| Input   | Action                           |
|---------|----------------------------------|
| Esc     | Close the window                 |

## Application Parameters

### Settings

| Parameter / Key               | Type              | Usage in Code                  | Description                    | Default Value |
|-------------------------------|-------------------|--------------------------------|--------------------------------|---------------|
| AppSettings.ClearColor        | VkClearColorValue | AppSettings::ClearColor        | Background color of the screen |               |

## Learning Objectives

- Understanding how to enable device feature for shader subgroup operations in Vulkan
- Learning how to use subgroup ballot and vote operations in shaders
- Implementing Conway's Game of Life using compute shaders
- Learning how to implement ping-pong buffering

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

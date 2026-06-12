# Using Staging Buffer

**Code Name:** UsingStagingBuffer

## Description

This example draws a blue octagon to the screen with using an additional staging buffer.

## Output

![](/Docs/ExampleMedia/Fundamentals/Basics/UsingStagingBuffer.png?raw=true)

## Controls

| Input | Action           |
|-------|------------------|
| Esc   | Close the window |

## Application Parameters

### Settings

| Parameter / Key        | Type              | Usage in Code           | Description                                            | Default Value |
|------------------------|-------------------|-------------------------|--------------------------------------------------------|---------------|
| AppSettings.ClearColor | VkClearColorValue | AppSettings::ClearColor | Specifies which color the screen will be cleared with. |               |


## Learning Objectives

- Demonstrate transferring data to the GPU using a staging buffer
- Exploring different kind of memory properties

## Shader Status

| Shader Type                                                    | Status             | Notes |
|----------------------------------------------------------------|--------------------|-------|
| [GLSL](/Shaders/Fundamentals/Basics/UsingStagingBuffer/glsl)   | :white_check_mark: |       |
| [HLSL](/Shaders/Fundamentals/Basics/UsingStagingBuffer/hlsl)   | :white_check_mark: |       |
| [Slang](/Shaders/Fundamentals/Basics/UsingStagingBuffer/slang) | :white_check_mark: |       |

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain

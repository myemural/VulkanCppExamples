# Handling Window Resizing

**Code Name:** SwapChainRecreation

## Description

In this example, some cubes are drawn on the screen. If the window size is changed, the swap chain object is recreated and the drawing is reapplied according to the new window size.

## Output

![](/Docs/ExampleMedia/Fundamentals/SwapChainsAndViewports/SwapChainRecreation.png?raw=true)

## Controls

| Input   | Action                      |
|---------|-----------------------------|
| Esc     | Close the window            |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                    | Default Value |
|------------------------------|-------------------|-------------------------------|--------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen |               |


## Learning Objectives

- Handling window resizing in Vulkan
- Recreating the swap chain

## Shader Status

| Shader Type                                                                     | Status             | Notes |
|---------------------------------------------------------------------------------|--------------------|-------|
| [GLSL](/Shaders/Fundamentals/SwapChainsAndViewports/SwapChainRecreation/glsl)   | :white_check_mark: |       |
| [HLSL](/Shaders/Fundamentals/SwapChainsAndViewports/SwapChainRecreation/hlsl)   | :white_check_mark: |       |
| [Slang](/Shaders/Fundamentals/SwapChainsAndViewports/SwapChainRecreation/slang) | :white_check_mark: |       |

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain
- VK_EXT_shader_viewport_index_layer

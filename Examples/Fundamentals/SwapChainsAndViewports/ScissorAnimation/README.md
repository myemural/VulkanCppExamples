# Scissor Animation

**Code Name:** ScissorAnimation

## Description

In this example, there is a constantly rotating scissor rectangle on the screen, and the scene is visible only from within this rectangle. The rest of the screen is black.

## Screenshots / Recordings

![](/Docs/ExampleMedia/Fundamentals/SwapChainsAndViewports/ScissorAnimation.png?raw=true)

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

- Setting scissor dynamically in pipeline
- Animating view on the screen with scissor

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

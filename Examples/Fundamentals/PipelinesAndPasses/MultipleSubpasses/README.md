# Using Multiple Subpasses and Input Attachments

**Code Name:** MultipleSubpasses

## Description

In this example, three cubes rotating at different speeds are drawn on the screen, and the depth buffer data for these three cubes is drawn visually on the fourth cube that drawn on the right side of the screen. Input attachment and multiple subpasses are used for this purpose.

## Screenshots / Recordings

![](/Docs/ExampleMedia/Fundamentals/PipelinesAndPasses/MultipleSubpasses.png?raw=true)

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

- Creating and using multiple subpasses
- Using input attachment in shaders
- Visualizing the depth image of the scene

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain

# Using Multiple Render Passes

**Code Name:** MultipleRenderPasses

## Description

In this example, a plane with a cloud texture moving into the background is drawn with one render pass. Then, a second render pass draws the cubes using a perspective view.

## Screenshots / Recordings

![](/Docs/ExampleMedia/Fundamentals/PipelinesAndPasses/MultipleRenderPasses.png?raw=true)

## Controls

| Input   | Action                      |
|---------|-----------------------------|
| W/A/S/D | Move the camera             |
| Mouse   | Look around with the camera |
| Esc     | Close the window            |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                              | Default Value |
|------------------------------|-------------------|-------------------------------|------------------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen           |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value                  |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera                      |               |
| AppSettings.CloudSpeed       | float             | AppSettings::CloudSpeed       | Moving speed of the clouds in background |               |


## Learning Objectives

- Creating and using multiple render passes
- Drawing an animated background scene

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain

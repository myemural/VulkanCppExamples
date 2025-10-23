# Instanced Rendering

**Code Name:** InstancedRendering

## Description

This example draws 20 rotating cubes on the screen using instanced rendering method.

## Screenshots / Recordings

![](/Docs/ExampleMedia/Fundamentals/Drawing3D/InstancedRendering.gif?raw=true)

## Controls

| Input   | Action                      |
|---------|-----------------------------|
| W/A/S/D | Move the camera             |
| Mouse   | Look around with the camera |
| Esc     | Close the window            |

## Application Parameters

### Settings

| Parameter / Key                | Type              | Usage in Code                   | Description                                  | Default Value |
|--------------------------------|-------------------|---------------------------------|----------------------------------------------|---------------|
| AppSettings.ClearColor         | VkClearColorValue | AppSettings::ClearColor         | Background color of the screen               |               |
| AppSettings.MouseSensitivity   | float             | AppSettings::MouseSensitivity   | Mouse sensitivity value                      |               |
| AppSettings.CameraSpeed        | float             | AppSettings::CameraSpeed        | Speed of the camera                          |               |
| AppSettings.FirstInstanceIndex | std::uint32_t     | AppSettings::FirstInstanceIndex | Index of the first instance to start drawing |               |


## Learning Objectives

- Using instanced rendering method to draw multiple same objects

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain

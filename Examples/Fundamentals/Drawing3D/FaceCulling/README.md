# FaceCulling

**Code Name:** BasicCameraControl

## Description

This example draws 10 static cubes on the screen using the user-specified cull mode and front face settings. You can see the differences by moving with the camera.

## Screenshots / Recordings

![](/Docs/ExampleMedia/Fundamentals/Drawing3D/FaceCulling.gif?raw=true)

## Controls

| Input   | Action                      |
|---------|-----------------------------|
| W/A/S/D | Move the camera             |
| Mouse   | Look around with the camera |
| Esc     | Close the window            |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                               | Default Value |
|------------------------------|-------------------|-------------------------------|-------------------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen            |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value                   |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera                       |               |
| AppSettings.CullMode         | VkCullModeFlags   | AppSettings::CullMode         | Cull mode value of the drawn objects      |               |
| AppSettings.FrontFace        | VkFrontFace       | AppSettings::FrontFace        | Fron face mode value of the drawn objects |               |


## Learning Objectives

- Understanding different cull mode settings
- Understanding different front face settings

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain

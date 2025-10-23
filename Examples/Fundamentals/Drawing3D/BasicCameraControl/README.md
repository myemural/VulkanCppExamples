# Basic Camera Control

**Code Name:** BasicCameraControl

## Description

This examples draws a cube to the screen and rotates it constantly. But this time also has a camera that controlling with WASD keys and mouse. With this camera you can move around the scene as you wish.

## Screenshots / Recordings

![](/Docs/ExampleMedia/Fundamentals/Drawing3D/BasicCameraControl.gif?raw=true)

## Controls

| Input   | Action                      |
|---------|-----------------------------|
| W/A/S/D | Move the camera             |
| Mouse   | Look around with the camera |
| Esc     | Close the window            |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                    | Default Value |
|------------------------------|-------------------|-------------------------------|--------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value        |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera            |               |


## Learning Objectives

- Moving in the scene with using keyboard and mouse
- Creating a virtual camera with changing the MVP matrix
- Basic input system implementation

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain

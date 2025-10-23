# Drawing a Cube

**Code Name:** DrawingCube

## Description

This example draws a cube to the screen and rotates it X and Y directions simultaneously to show depth testing, depth image and basic 3D rendering. Also, it shows perspective projection and MVP matrix usage.

## Screenshots / Recordings

![](/Docs/ExampleMedia/Fundamentals/Drawing3D/DrawingCube.gif?raw=true)

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

- Enabling depth testing
- Drawing a textured 3D cube to the scene
- Transforming the cube with updating MVP matrix
- Simulating the perspective projection

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain

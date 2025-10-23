# Depth Testing Operations

**Code Name:** DepthTestingOperations

## Description

This examples draws two planes and two cubes to the screen. Then applies depth test, depth write and depth operation related settings which getting from user.

## Screenshots / Recordings

![](/Docs/ExampleMedia/Fundamentals/Drawing3D/DepthTestingOperations.gif?raw=true)

## Controls

| Input   | Action                      |
|---------|-----------------------------|
| W/A/S/D | Move the camera             |
| Mouse   | Look around with the camera |
| Esc     | Close the window            |

## Application Parameters

### Settings

| Parameter / Key               | Type              | Usage in Code                  | Description                            | Default Value      |
|-------------------------------|-------------------|--------------------------------|----------------------------------------|--------------------|
| AppSettings.ClearColor        | VkClearColorValue | AppSettings::ClearColor        | Background color of the screen         |                    |
| AppSettings.MouseSensitivity  | float             | AppSettings::MouseSensitivity  | Mouse sensitivity value                |                    |
| AppSettings.CameraSpeed       | float             | AppSettings::CameraSpeed       | Speed of the camera                    |                    |
| AppSettings.DepthTestEnabled  | bool              | AppSettings::DepthTestEnabled  | Is depth test enabled or not           | true               |
| AppSettings.DepthWriteEnabled | bool              | AppSettings::DepthWriteEnabled | Is depth write enabled or not          | true               |
| AppSettings.DepthCompareOp    | VkCompareOp       | AppSettings::DepthCompareOp    | Compare operation of the depth testing | VK_COMPARE_OP_LESS |


## Learning Objectives

- Experiencing the effect of using different kind of depth compare operations in depth testing
- Experiencing the effect of enabling/disabling depth test and depth write operations in the pipeline

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain

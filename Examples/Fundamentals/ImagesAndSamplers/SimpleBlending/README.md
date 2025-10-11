# Drawing Transparent Texture to Quads

**Code Name:** SimpleBlending

## Description

This examples draws 4 quads to the screen. Then it applies a transparent leaf texture to all of them to test simple alpha blending.

## Screenshots / Recordings

![](/Docs/ExampleMedia/Fundamentals/ImagesAndSamplers/SimpleBlending.png?raw=true)

## Controls

| Input | Action                       |
|-------|------------------------------|
| Esc   | Close the window             |

## Application Parameters

### Settings

| Parameter / Key                  | Type              | Usage in Code                     | Description                    | Default Value |
|----------------------------------|-------------------|-----------------------------------|--------------------------------|---------------|
| AppSettings.ClearColor           | VkClearColorValue | AppSettings::ClearColor           | Background color of the screen |               |


## Learning Objectives

- Loading and using transparent textures
- Simple blending operations in Vulkan

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain

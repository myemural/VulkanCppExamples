# Using Different Wrap and Filtering Modes

**Code Name:** WrapAndFilteringModes

## Description

This example draws 4 quads to the screen. Then it applies them different samplers that have different wrap and filtering modes.

## Screenshots / Recordings

![](/Docs/ExampleMedia/Fundamentals/ImagesAndSamplers/WrapAndFilteringModes.png?raw=true)

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

- Creating and applying multiple samplers
- Using multiple samplers in shaders

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain

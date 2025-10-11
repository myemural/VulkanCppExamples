# Combined Image Sampler

**Code Name:** CombinedImageSampler

## Description

This example draws a single quad to the screen, but applies a texture on it with using combined images sampler.

## Screenshots / Recordings

![](/Docs/ExampleMedia/Fundamentals/ImagesAndSamplers/CombinedImageSampler.png?raw=true)

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

- Creating combined image sampler descriptor
- Using combined image sampler descriptors in shaders

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain

# Using Multiple Textures

**Code Name:** UsingMultipleTextures

## Description

This examples draws 3 textured quads to the screen. Top quads have different textures and bottom quad has multiple textures that mixes of these two textures.

## Screenshots / Recordings

![](/Docs/ExampleMedia/Fundamentals/ImagesAndSamplers/UsingMultipleTextures.png?raw=true)

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

- Creating and using multiple images and image views
- Using multiple textures in shaders
- Mixing multiple textures in shaders

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain

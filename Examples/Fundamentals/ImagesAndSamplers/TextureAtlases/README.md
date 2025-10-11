# Using Texture Atlases

**Code Name:** TextureAtlases

## Description

This example draws 4 quads to the screen. Then it applies different textures from a texture atlas to each of them.

## Screenshots / Recordings

![](/Docs/ExampleMedia/Fundamentals/ImagesAndSamplers/TextureAtlases.png?raw=true)

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

- Using texture atlases in Vulkan (loading one image for multiple textures)
- Managing texture coordinates

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain

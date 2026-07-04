# Checkerboard Texture Generator

**Code Name:** CheckerboardGenerator

## Description

In this example, a checkerboard texture is first generated with a compute shader and then applied to the spheres in the scene. The user can also specify the size and tile size of the texture to be generated.

## Output

![](/Docs/ExampleMedia/Fundamentals/ComputeShaders/CheckerboardGenerator.png?raw=true)

## Controls

| Input   | Action                      |
|---------|-----------------------------|
| W/A/S/D | Move the camera             |
| Mouse   | Look around with the camera |
| Esc     | Close the window            |

## Application Parameters

### Settings

| Parameter / Key                     | Type              | Usage in Code                        | Description                                     | Default Value |
|-------------------------------------|-------------------|--------------------------------------|-------------------------------------------------|---------------|
| AppSettings.ClearColor              | VkClearColorValue | AppSettings::ClearColor              | Background color of the screen                  |               |
| AppSettings.MouseSensitivity        | float             | AppSettings::MouseSensitivity        | Mouse sensitivity value                         |               |
| AppSettings.CameraSpeed             | float             | AppSettings::CameraSpeed             | Speed of the camera                             |               |
| AppSettings.CheckerboardTextureSize | std::uint32_t     | AppSettings::CheckerboardTextureSize | Width and height value of the generated texture |               |
| AppSettings.TileSize                | int               | AppSettings::TileSize                | Tile size of the checkerboard texture           |               |


## Learning Objectives

- Creating a texture from compute shader
- Placing generated texture to the objects

## Shader Status

| Shader Type                                                               | Status             | Notes |
|---------------------------------------------------------------------------|--------------------|-------|
| [GLSL](/Shaders/Fundamentals/ComputeShaders/CheckerboardGenerator/glsl)   | :white_check_mark: |       |
| [HLSL](/Shaders/Fundamentals/ComputeShaders/CheckerboardGenerator/hlsl)   | :white_check_mark: |       |
| [Slang](/Shaders/Fundamentals/ComputeShaders/CheckerboardGenerator/slang) | :white_check_mark: |       |

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain

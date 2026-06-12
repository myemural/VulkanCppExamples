# Fundamental Diffuse and Specular Model of the Disney BRDF

**Code Name:** DisneyDiffuseSpecular

## Description

In this example, Disney BRDF's basic diffuse and specular models are implemented along with IBL. The roughness, metallic, subsurface, specular, and specular tint parameters of Disney's material model are tested.

## Output

![](/Docs/ExampleMedia/PhysicallyBasedRendering/DisneyPrincipledBrdf/DisneyDiffuseSpecular.png?raw=true)

## Controls

| Input   | Action                      |
|---------|-----------------------------|
| W/A/S/D | Move the camera             |
| Mouse   | Look around with the camera |
| Esc     | Close the window            |
| 0       | IBL disabled                |
| 1       | IBL enabled                 |

## Application Parameters

### Settings

| Parameter / Key                  | Type              | Usage in Code                     | Description                                    | Default Value |
|----------------------------------|-------------------|-----------------------------------|------------------------------------------------|---------------|
| AppSettings.ClearColor           | VkClearColorValue | AppSettings::ClearColor           | Background color of the screen                 |               |
| AppSettings.MouseSensitivity     | float             | AppSettings::MouseSensitivity     | Mouse sensitivity value                        |               |
| AppSettings.CameraSpeed          | float             | AppSettings::CameraSpeed          | Speed of the camera                            |               |

## Learning Objectives

- Implementing Disney BRDF's roughness and metallic parameters with IBL in Vulkan
- Implementing Disney BRDF's subsurface parameter with IBL in Vulkan
- Implementing Disney BRDF's specular and specular tint parameters with IBL in Vulkan

## Shader Status

| Shader Type | Status             | Notes                |
|-------------|--------------------|----------------------|
| GLSL        | :white_check_mark: |                      |
| HLSL        | :x:                | Will be implemented. |
| Slang       | :x:                | Will be implemented. |

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain

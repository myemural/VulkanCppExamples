# Manual Mipmap LOD Control

**Code Name:** ManualMipmapping

## Description

In this example, a mipmap level is manually applied to the diffuse textures of the objects drawn on the screen using the shader. The user can select this mipmap level by pressing keys 0-7.

## Output

![](/Docs/ExampleMedia/RealTimeLighting/TextureSamplingAndFiltering/ManualMipmapping.png?raw=true)

## Controls

| Input           | Action                                                 |
|-----------------|--------------------------------------------------------|
| W/A/S/D         | Move the camera                                        |
| Mouse           | Look around with the camera                            |
| Esc             | Close the window                                       |
| 0/1/2/3/4/5/6/7 | Set current mipmap level (between 0-7) of the textures |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                      | Default Value |
|------------------------------|-------------------|-------------------------------|----------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen   |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value          |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera              |               |
| AppSettings.LightDirection   | glm:vec3          | AppSettings::LightDirection   | Direction of the light           |               |
| AppSettings.LightColor       | glm:vec3          | AppSettings::LightColor       | Color of the light (RGB)         |               |
| AppSettings.AmbientStrength  | float             | AppSettings::AmbientStrength  | Ambient strength of the material |               |
| AppSettings.SpecularStrength | float             | AppSettings::SpecularStrength | Specular strength factor         |               |
| AppSettings.Shininess        | float             | AppSettings::Shininess        | Shininess exponent               |               |


## Learning Objectives

- Understanding manual mipmap level selection mechanism when sampling images in shaders
- Changing current mipmap level via keyboard input

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

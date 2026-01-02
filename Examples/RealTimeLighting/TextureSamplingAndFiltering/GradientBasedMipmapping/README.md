# Gradient-Based LOD Control with Anisotropy

**Code Name:** GradientBasedMipmapping

## Description

In this example, gradient-based manual LOD control is applied via shader to objects drawn on the screen that have anisotropic filtering applied to their textures. Additionally, the diffuse texture is mixed with red color depending on the object's distance to the camera for showing the LOD level better.

## Output

![](/Docs/ExampleMedia/RealTimeLighting/TextureSamplingAndFiltering/GradientBasedMipmapping.png?raw=true)

## Controls

| Input           | Action                                                 |
|-----------------|--------------------------------------------------------|
| W/A/S/D         | Move the camera                                        |
| Mouse           | Look around with the camera                            |
| Esc             | Close the window                                       |

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
| AppSettings.MinMipmapLevel   | float             | AppSettings::MinMipmapLevel   | Minimum mipmap level             |               |


## Learning Objectives

- Implementing gradient-based manual mipmap level control in shaders
- Setting different color for objects that have different mipmap levels (for debugging)

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

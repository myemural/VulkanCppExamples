# Using Multiple Light Sources

**Code Name:** MultipleLightSources

## Description

In this example, objects of different colors are drawn on the scene, and lighting is provided by multiple lights which can be directional light, point light or spotlight (soft-cutoff). Light count provided to the shaders with a specialization constant.

## Output

![](/Docs/ExampleMedia/RealTimeLighting/LightSources/MultipleLightSources.png?raw=true)

## Controls

| Input   | Action                      |
|---------|-----------------------------|
| W/A/S/D | Move the camera             |
| Mouse   | Look around with the camera |
| Esc     | Close the window            |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                                 | Default Value |
|------------------------------|-------------------|-------------------------------|---------------------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen              |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value                     |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera                         |               |
| AppSettings.LightColor       | glm:vec3          | AppSettings::LightColor       | Color of the light (RGB)                    |               |
| AppSettings.AmbientStrength  | float             | AppSettings::AmbientStrength  | Ambient strength of the material            |               |
| AppSettings.SpecularStrength | float             | AppSettings::SpecularStrength | Specular strength factor                    |               |
| AppSettings.Shininess        | float             | AppSettings::Shininess        | Shininess exponent                          |               |
| AppSettings.InnerCutoffAngle | float             | AppSettings::InnerCutoffAngle | Inner cutoff angle of the spotlight         |               |
| AppSettings.OuterCutoffAngle | float             | AppSettings::OuterCutoffAngle | Outer cutoff angle of the spotlight         |               |
| AppSettings.ConstantFactor   | float             | AppSettings::ConstantFactor   | Constant factor in the attenuation formula  |               |
| AppSettings.LinearFactor     | float             | AppSettings::LinearFactor     | Linear factor in the attenuation formula    |               |
| AppSettings.QuadraticFactor  | float             | AppSettings::QuadraticFactor  | Quadratic factor in the attenuation formula |               |


## Learning Objectives

- Implementing various and numerous light sources behavior in the shaders
- Sending multiple light data and count via storage buffers and specialization constants

## Shader Status

| Shader Type                                                              | Status             | Notes                |
|--------------------------------------------------------------------------|--------------------|----------------------|
| [GLSL](/Shaders/RealTimeLighting/LightSources/MultipleLightSources/glsl) | :white_check_mark: |                      |
| [HLSL](/Shaders/RealTimeLighting/LightSources/MultipleLightSources/hlsl) | :white_check_mark: |                      |
| Slang                                                                    | :x:                | Will be implemented. |

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain

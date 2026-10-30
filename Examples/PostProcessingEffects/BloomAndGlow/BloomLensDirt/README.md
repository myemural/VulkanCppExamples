# Bloom with Lens Dirt and Anamorphic Stretch

**Code Name:** BloomLensDirt

## Description

In this example, the physically based mip-chain bloom has been extended with two common artistic lens effects. A lens dirt texture modulates the bloom, so dust and smudges on the lens become visible around bright areas of the screen. The anamorphic stretch widens the upsample filter only horizontally on every level of the mip-chain.

## Output

![](/Docs/ExampleMedia/PostProcessingEffects/BloomAndGlow/BloomLensDirt.png?raw=true)

## Controls

| Input   | Action                                 |
|---------|----------------------------------------|
| W/A/S/D | Move the camera                        |
| Mouse   | Look around with the camera            |
| Esc     | Close the window                       |
| Space   | Turn on/off bloom effect               |
| B       | Switch debug view (final / bloom only) |
| Q       | Increase exposure value                |
| E       | Decrease exposure value                |
| R       | Increase bloom strength value          |
| F       | Decrease bloom strength value          |
| T       | Increase lens dirt intensity value     |
| G       | Decrease lens dirt intensity value     |
| Y       | Increase anamorphic stretch value      |
| H       | Decrease anamorphic stretch value      |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                      | Default Value |
|------------------------------|-------------------|-------------------------------|----------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen   |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value          |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera              |               |

## Learning Objectives

- Understanding how to apply a lens dirt texture to the bloom effect
- Understanding how to create an anamorphic (horizontally stretched) bloom by scaling the upsample filter

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

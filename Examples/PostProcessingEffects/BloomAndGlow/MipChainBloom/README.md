# Mip-Chain Bloom (Physically Based, Downsample/Upsample)

**Code Name:** MipChainBloom

## Description

In this example, a physically based bloom effect has been implemented with a mip chain. The HDR scene color is downsampled into a chain of progressively smaller mips with a 13-tap filter (the first pass uses Karis average to suppress fireflies), then the chain is upsampled back with a 3x3 tent filter and additive blending.

## Output

![](/Docs/ExampleMedia/PostProcessingEffects/BloomAndGlow/MipChainBloom.png?raw=true)

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

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                      | Default Value |
|------------------------------|-------------------|-------------------------------|----------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen   |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value          |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera              |               |

## Learning Objectives

- Understanding how to implement a physically based (threshold-less) bloom effect with a mip chain
- Downsampling with the 13-tap filter and suppressing fireflies with Karis average
- Upsampling with a 3x3 tent filter and accumulating the mip chain with additive blending

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

# Using Equirectangular HDR Images as Skybox

**Code Name:** EquirectangularHdrSkybox

## Description

In this example, an equirectangular HDR image is first loaded, then converted into a cubemap, and finally used as a skybox.

## Output

![](/Docs/ExampleMedia/PhysicallyBasedRendering/IblAndReflections/EquirectangularHdrSkybox.png?raw=true)

## Controls

| Input        | Action                     |
|--------------|----------------------------|
| Mouse Move   | Rotataing the orbit camera |
| Mouse Scroll | Zooming the orbit camera   |
| Esc          | Close the window           |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                      | Default Value |
|------------------------------|-------------------|-------------------------------|----------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen   |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value          |               |
| AppSettings.CameraZoomSpeed  | float             | AppSettings::CameraZoomSpeed  | Zoom speed of the camera         |               |

## Learning Objectives

- Loading HDR images
- Converting an equirectangular HDR image to a cubemap
- Using equirectangular HDR images as a skybox

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

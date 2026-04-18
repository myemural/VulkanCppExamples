# Visibility Check with Occlusion Queries

**Code Name:** OcclusionQuery

## Description

In this example, a cube is drawn with a sphere inside it. The occlusion query periodically checks whether fragments of the sphere are visible.

## Output

![](/Docs/ExampleMedia/Fundamentals/QueriesAndPerformance/OcclusionQuery.png?raw=true)

**Console Output**
```bash
Sphere visibility: false
Number of visible fragments: 0
Sphere visibility: false
Number of visible fragments: 0
Sphere visibility: true
Number of visible fragments: 125188
...
```

## Controls

| Input   | Action                      |
|---------|-----------------------------|
| W/A/S/D | Move the camera             |
| Mouse   | Look around with the camera |
| Esc     | Close the window            |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                    | Default Value |
|------------------------------|-------------------|-------------------------------|--------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value        |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera            |               |


## Learning Objectives

- Using occlusion queries in Vulkan
- Checking visibility of an object via occlusion queries

## Shader Status

| Shader Type | Status             | Notes |
|-------------|--------------------|-------|
| GLSL        | :white_check_mark: |       |
| HLSL        | :white_check_mark: |       |
| Slang       | :white_check_mark: |       |

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain

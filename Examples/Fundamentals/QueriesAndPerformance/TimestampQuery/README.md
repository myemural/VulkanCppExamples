# Measuring The GPU Time with Timestamp Queries

**Code Name:** TimestampQuery

## Description

In this example, 20 cubes in four different groups are drawn on the screen using four different pipelines. The first two pipelines are drawn in fill polygon mode, while the last two are drawn as wireframes. Finally, the runtimes of these pipelines are measured using timestamp queries.

## Output

![](/Docs/ExampleMedia/Fundamentals/QueriesAndPerformance/TimestampQuery.png?raw=true)

**Console Output**
```bash
-----------------
Pipeline 1: 23177ns
Pipeline 2: 12760ns
Pipeline 3: 8020ns
Pipeline 4: 9895ns
-----------------
-----------------
Pipeline 1: 5156ns
Pipeline 2: 5052ns
Pipeline 3: 4635ns
Pipeline 4: 9635ns
-----------------
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

- Using timestamp queries in Vulkan
- Measuring the GPU time of the commands

## Shader Status

| Shader Type                                                               | Status             | Notes |
|---------------------------------------------------------------------------|--------------------|-------|
| [GLSL](/Shaders/Fundamentals/QueriesAndPerformance/TimestampQuery/glsl)   | :white_check_mark: |       |
| [HLSL](/Shaders/Fundamentals/QueriesAndPerformance/TimestampQuery/hlsl)   | :white_check_mark: |       |
| [Slang](/Shaders/Fundamentals/QueriesAndPerformance/TimestampQuery/slang) | :white_check_mark: |       |

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain

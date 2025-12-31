# Measuring Pipeline Statistics

**Code Name:** PipelineStatisticsQuery

## Description

In this example, 20 cubes in four different groups are drawn on the screen using four different pipelines. Then, using the pipeline statistics query, information about these 4 pipelines is printed to the console.

## Output

![](/Docs/ExampleMedia/Fundamentals/QueriesAndPerformance/PipelineStatisticsQuery.png?raw=true)

**Console Output**
```bash
##################### PIPELINE STATISTICS #####################
----------------- Pipeline 1 -------------------
Input Assembly Vertices: 108
Input Assembly Primitives: 36
Vertex Shader Invocations: 72
Fragment Shader Invocations: 34084
-------------------------------------------------------------
----------------- Pipeline 2 -------------------
Input Assembly Vertices: 252
Input Assembly Primitives: 84
Vertex Shader Invocations: 168
Fragment Shader Invocations: 28748
-------------------------------------------------------------
----------------- Pipeline 3 -------------------
Input Assembly Vertices: 144
Input Assembly Primitives: 48
Vertex Shader Invocations: 96
Fragment Shader Invocations: 8396
-------------------------------------------------------------
----------------- Pipeline 4 -------------------
Input Assembly Vertices: 216
Input Assembly Primitives: 72
Vertex Shader Invocations: 144
Fragment Shader Invocations: 10732
-------------------------------------------------------------
###############################################################
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

- Using pipeline statistics query in Vulkan
- Measuring input assembly vertices and primitives count of a pipeline
- Measuring shader invocation counts of a pipeline

## Shader Status

| Shader Type | Status             | Notes                |
|-------------|--------------------|----------------------|
| GLSL        | :white_check_mark: |                      |
| HLSL        | :white_check_mark: |                      |
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

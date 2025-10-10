# Drawing with Different Topologies

**Code Name:** DrawingWithDifferentTopology

## Description

This example draws 2 quads to the screen to demonstrate primitive restart index usage. Also, user can set polygon mode and line width (if line mode has been selected).

## Screenshots / Recordings

![](/Docs/ExampleMedia/Fundamentals/Basics/DrawingWithDifferentTopology.png?raw=true)

## Controls

| Input | Action           |
|-------|------------------|
| Esc   | Close the window |

## Application Parameters

### Settings

| Parameter / Key                     | Type              | Usage in Code                        | Description                                            | Default Value |
|-------------------------------------|-------------------|--------------------------------------|--------------------------------------------------------|---------------|
| AppSettings.ClearColor              | VkClearColorValue | AppSettings::ClearColor              | Specifies which color the screen will be cleared with. |               |
| AppSettings.PrimitiveRestartEnabled | bool              | AppSettings::PrimitiveRestartEnabled | Toggle primitive restart index setting for pipeline.   |               |
| AppSettings.PolygonMode             | VkPolygonMode     | AppSettings::PolygonMode             | Specifies which polygon mode will be applied.          |               |
| AppSettings.LineWidth               | float             | AppSettings::LineWidth               | Specifies line width (if line mode has been selected). |               |


## Learning Objectives

- Changing some rasterization related values in pipeline
- Drawing lines or points instead of filled drawings
- Understanding the use of primitive restart index
- Adjusting line width in line drawing mode

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain

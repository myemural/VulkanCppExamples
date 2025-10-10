# Drawing a Quad with Indices

**Code Name:** DrawingQuad

## Description

This example draws a quad to the screen with using index buffer in addition to vertex buffer.

## Screenshots / Recordings

![](/Docs/ExampleMedia/Fundamentals/Basics/DrawingQuad.png?raw=true)

## Controls

| Input | Action           |
|-------|------------------|
| Esc   | Close the window |

## Application Parameters

### Settings

| Parameter / Key        | Type              | Usage in Code           | Description                                            | Default Value |
|------------------------|-------------------|-------------------------|--------------------------------------------------------|---------------|
| AppSettings.ClearColor | VkClearColorValue | AppSettings::ClearColor | Specifies which color the screen will be cleared with. |               |


## Learning Objectives

- Creating a index buffer and bind it via command
- Doing indexed drawing

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain

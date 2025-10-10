# Drawing a Single Color Triangle

**Code Name:** DrawingSingleColorTriangle

## Description

This example shows hot to draw a single colored triangle to the screen to demonstrate pipeline and simple vertex and fragment shader usages.

## Screenshots / Recordings

![](/Docs/ExampleMedia/Fundamentals/Basics/DrawingSingleColorTriangle.png?raw=true)

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

- Creating and managing shader modules
- Writing simple vertex and fragment shaders
- Creating a simple graphics pipeline
- Binding pipeline and drawing to framebuffers using it

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain

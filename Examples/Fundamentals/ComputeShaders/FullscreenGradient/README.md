# Fullscreen Moving Gradient Generator

**Code Name:** FullscreenGradient

## Description

In this example, a full-screen quad is drawn on the screen, and the gradient output from the compute shader is placed on it. This gradient moves over time.

## Output

![](/Docs/ExampleMedia/Fundamentals/ComputeShaders/FullscreenGradient.png?raw=true)

## Controls

| Input | Action           |
|-------|------------------|
| Esc   | Close the window |

## Application Parameters

### Settings

None

## Learning Objectives

- Creating compute pipelines
- Generating dynamic fullscreen gradient image from compute shader
- Drawing results of the compute shader to the screen via storage images

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain

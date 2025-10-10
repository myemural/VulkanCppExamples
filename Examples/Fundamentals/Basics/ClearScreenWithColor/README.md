# Clear Screen with a Color

**Code Name:** ClearScreenWithColor

## Description

This example shows how to create a render pass and framebuffer, along with basic swap chain creation and synchronization in Vulkan, and how to clear the screen with a simple color.

## Screenshots / Recordings

![](/Docs/ExampleMedia/Fundamentals/Basics/ClearScreenWithColor.png?raw=true)

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

- Obtaining surface information from the window system
- Creating a logical device and obtaining a queue from it
- Creating a swap chain and obtaining image views dependant to it
- Creating a simple render pass and framebuffer
- Creating a command pool and creating command buffers equal to the number of framebuffers
- Filling command buffers with commands
- Creating and managing basic synchronization objects (fence, semaphore)
- Pushing command buffers to the queue and presenting framebuffer results to the screen

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain

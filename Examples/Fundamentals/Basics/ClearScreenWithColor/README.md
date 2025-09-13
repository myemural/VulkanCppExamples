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

### Window Parameters

| Parameter / Key    | Type          | Usage in Code             | Description                             | Rewritable? |
|--------------------|---------------|---------------------------|-----------------------------------------|-------------|
| Window.Width       | std::uint32_t | WindowParams::Width       | Initial width of the window (in pixel)  | No          |
| Window.Height      | std::uint32_t | WindowParams::Height      | Initial height of the window (in pixel) | No          |
| Window.Title       | std::string   | WindowParams::Title       | Title of the window                     | No          |
| Window.Resizable   | bool          | WindowParams::Resizable   | Whether the window is resizable or not  | No          |
| Window.SampleCount | unsigned int  | WindowParams::SampleCount | Sample count of the window              | No          |

### Vulkan Parameters

| Parameter / Key        | Type                    | Usage in Code                 | Description                                                                        | Rewritable? |
|------------------------|-------------------------|-------------------------------|------------------------------------------------------------------------------------|-------------|
| Vulkan.AppCreateConfig | ApplicationCreateConfig | VulkanParams::AppCreateConfig | Contains Vulkan related configs like app name, Vulkan version, render loop ms etc. | No          |

### Project Constants

| Parameter / Key           | Type          | Usage in Code                    | Description                                                                             | Rewritable? |
|---------------------------|---------------|----------------------------------|-----------------------------------------------------------------------------------------|-------------|
| Project.MaxFramesInFlight | std::uint32_t | ProjectParams::MaxFramesInFlight | Specifies the maximum number of images to be processed at each step in the render loop. | No          |

### Project Customizable Settings

| Parameter / Key           | Type          | Usage in Code                    | Description                                                                             | Rewritable? |
|---------------------------|---------------|----------------------------------|-----------------------------------------------------------------------------------------|-------------|
| Project.ClearColor | VkClearColorValue | ProjectParams::ClearColor | Specifies which color the screen will be cleared with. | Yes         |


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

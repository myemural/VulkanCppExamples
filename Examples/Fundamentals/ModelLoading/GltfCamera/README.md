# Camera Usage with glTF

**Code Name:** GltfCamera

## Description

In this example, a glTF quad model rendered on the screen with specified camera node information (perspective or orthographic).

## Screenshots / Recordings

![](/Docs/ExampleMedia/Fundamentals/ModelLoading/GltfCamera.png?raw=true)

## Controls

| Input   | Action                      |
|---------|-----------------------------|
| Esc     | Close the window            |

## Application Parameters

### Settings

| Parameter / Key                 | Type              | Usage in Code                    | Description                                         | Default Value |
|---------------------------------|-------------------|----------------------------------|-----------------------------------------------------|---------------|
| AppSettings.ClearColor          | VkClearColorValue | AppSettings::ClearColor          | Background color of the screen                      |               |
| AppSettings.IsPerspectiveCamera | bool              | AppSettings::IsPerspectiveCamera | Whether camera is perspective or not (orthographic) |               |


## Learning Objectives

- Applying camera settings which defined in the node of the glTF file

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain

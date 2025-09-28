# Change Square Color with Keyboard Input

**Code Name:** BasicPushConstants

## Description

This example demonstrate the basic push constants usage with changing color of the square with keyboard inputs 1, 2, 3.

## Screenshots / Recordings

![](/Docs/ExampleMedia/Fundamentals/DescriptorSets/BasicPushConstants.png?raw=true)

## Controls

| Input | Action                       |
|-------|------------------------------|
| Esc   | Close the window             |
| 1     | Change square color to red   |
| 2     | Change square color to green |
| 3     | Change square color to blue  |

## Application Parameters

### Settings

| Parameter / Key                  | Type              | Usage in Code                     | Description                    | Default Value |
|----------------------------------|-------------------|-----------------------------------|--------------------------------|---------------|
| AppSettings.ClearColor           | VkClearColorValue | AppSettings::ClearColor           | Background color of the screen |               |


## Learning Objectives

- Creating push constants and associate them with pipeline layouts
- Updating push constants constantly
- Getting value of the push constants from shaders

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain

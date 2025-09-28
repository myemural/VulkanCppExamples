# Rotating and Scaling a Square Constantly

**Code Name:** Transformation2dWithUB

## Description

This example draws a square in the center of the screen. Then rotates it in clockwise, also scaling up and down constantly.

## Screenshots / Recordings

![](/Docs/ExampleMedia/Fundamentals/DescriptorSets/Transformation2dWithUB.gif?raw=true)

## Controls

| Input | Action           |
|-------|------------------|
| Esc   | Close the window |

## Application Parameters

### Settings

| Parameter / Key                  | Type              | Usage in Code                     | Description                    | Default Value |
|----------------------------------|-------------------|-----------------------------------|--------------------------------|---------------|
| AppSettings.ClearColor           | VkClearColorValue | AppSettings::ClearColor           | Background color of the screen |               |


## Learning Objectives

- Transforming 2D objects via model matrices
- Updating model matrices constantly using a uniform buffer

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain

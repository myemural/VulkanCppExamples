# Changing Color of a Triangle with Uniform Buffer

**Code Name:** ChangingColorWithUB

## Description

This example draws a single triangle to the screen. Then it tries to change color via a Uniform Buffer(UB).

## Screenshots / Recordings

![](/Docs/ExampleMedia/Fundamentals/DescriptorSets/ChangingColorWithUB.png?raw=true)

## Controls

| Input | Action           |
|-------|------------------|
| Esc   | Close the window |

## Application Parameters

### Settings

| Parameter / Key           | Type              | Usage in Code              | Description                    | Default Value |
|---------------------------|-------------------|----------------------------|--------------------------------|---------------|
| AppSettings.ClearColor    | VkClearColorValue | AppSettings::ClearColor    | Background color of the screen |               |
| AppSettings.TriangleColor | Color3            | AppSettings::TriangleColor | Triangle color                 |               |


## Learning Objectives

- Creating a descriptor pool
- Creating descriptor set layouts and descriptor sets
- Associate descriptor set layouts with pipline layouts
- Update descriptor sets
- Get descriptor sets from shaders and use them

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain

# Rotating and Scaling a Square Constantly

**Code Name:** Transformation2dWithUB

## Description

This example draws a square in the center of the screen. Then rotates it in clockwise, also scaling up and down constantly.

## Output

![](/Docs/ExampleMedia/Fundamentals/DescriptorSets/Transformation2dWithUB.png?raw=true)

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

## Shader Status

| Shader Type                                                                | Status             | Notes |
|----------------------------------------------------------------------------|--------------------|-------|
| [GLSL](/Shaders/Fundamentals/DescriptorSets/Transformation2dWithUB/glsl)   | :white_check_mark: |       |
| [HLSL](/Shaders/Fundamentals/DescriptorSets/Transformation2dWithUB/hlsl)   | :white_check_mark: |       |
| [Slang](/Shaders/Fundamentals/DescriptorSets/Transformation2dWithUB/slang) | :white_check_mark: |       |

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain

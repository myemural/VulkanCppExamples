# Multiple Transform with Descriptor Arrays

**Code Name:** ArrayOfUB

## Description

This example draws 4 squares to the screen. Then it applies different transformations to each one constantly.

## Output

![](/Docs/ExampleMedia/Fundamentals/DescriptorSets/ArrayOfUB.png?raw=true)

## Controls

| Input | Action                       |
|-------|------------------------------|
| Esc   | Close the window             |

## Application Parameters

### Settings

| Parameter / Key                  | Type              | Usage in Code                     | Description                    | Default Value |
|----------------------------------|-------------------|-----------------------------------|--------------------------------|---------------|
| AppSettings.ClearColor           | VkClearColorValue | AppSettings::ClearColor           | Background color of the screen |               |


## Learning Objectives

- Creating a array of uniform buffers
- Using array of uniform buffers in shaders

## Shader Status

| Shader Type                                                   | Status             | Notes |
|---------------------------------------------------------------|--------------------|-------|
| [GLSL](/Shaders/Fundamentals/DescriptorSets/ArrayOfUB/glsl)   | :white_check_mark: |       |
| [HLSL](/Shaders/Fundamentals/DescriptorSets/ArrayOfUB/hlsl)   | :white_check_mark: |       |
| [Slang](/Shaders/Fundamentals/DescriptorSets/ArrayOfUB/slang) | :white_check_mark: |       |

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain

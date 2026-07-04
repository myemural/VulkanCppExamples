# Using Different UBs for Different Areas of the Screen

**Code Name:** MultipleUniformBuffers

## Description

This example draws 4 triangles to the different areas of the screen. Then it applies different Uniform Buffers(UBs) to different areas of the screen.

## Output

![](/Docs/ExampleMedia/Fundamentals/DescriptorSets/MultipleUniformBuffers.png?raw=true)

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

- Creating multiple descriptor sets that each related to a uniform buffer
- Using multiple descriptor sets in shaders

## Shader Status

| Shader Type                                                                | Status             | Notes |
|----------------------------------------------------------------------------|--------------------|-------|
| [GLSL](/Shaders/Fundamentals/DescriptorSets/MultipleUniformBuffers/glsl)   | :white_check_mark: |       |
| [HLSL](/Shaders/Fundamentals/DescriptorSets/MultipleUniformBuffers/hlsl)   | :white_check_mark: |       |
| [Slang](/Shaders/Fundamentals/DescriptorSets/MultipleUniformBuffers/slang) | :white_check_mark: |       |

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain

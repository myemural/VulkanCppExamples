# Drawing with Different Topologies

**Code Name:** DrawingWithDifferentTopology

## Description

This example draws 2 quads to the screen to demonstrate primitive restart index usage. Also, user can set polygon mode and line width (if line mode has been selected).

## Screenshots / Recordings

![](/Docs/ExampleMedia/Fundamentals/Basics/DrawingWithDifferentTopology.png?raw=true)

## Controls

| Input | Action           |
|-------|------------------|
| Esc   | Close the window |

## Application Parameters

### Constants

| Parameter / Key                     | Type           | Usage in Code                        | Description                                                  | Default Value        |
|-------------------------------------|----------------|--------------------------------------|--------------------------------------------------------------|----------------------|
| AppConstants.MaxFramesInFlight      | std::uint32_t  | AppConstants::MaxFramesInFlight      | Maximum number of images to be processed in the render loop. | 2                    |
| AppConstants.BaseShaderType         | ShaderBaseType | AppConstants::BaseShaderType         | Base shader type of the SPIR-V shader.                       | ShaderBaseType::GLSL |
| AppConstants.MainVertexShaderFile   | std::string    | AppConstants::MainVertexShaderFile   | Main vertex shader file path.                                | "triangle.vert.spv"  |
| AppConstants.MainFragmentShaderFile | std::string    | AppConstants::MainFragmentShaderFile | Main fragment shader file path.                              | "triangle.frag.spv"  |
| AppConstants.MainVertexShaderKey    | std::string    | AppConstants::MainVertexShaderKey    | Main vertex shader key.                                      | "vertMain"           |
| AppConstants.MainFragmentShaderKey  | std::string    | AppConstants::MainFragmentShaderKey  | Main fragment Shader key                                     | "fragMain"           |

### Settings

| Parameter / Key                     | Type              | Usage in Code                        | Description                                            | Default Value |
|-------------------------------------|-------------------|--------------------------------------|--------------------------------------------------------|---------------|
| AppSettings.ClearColor              | VkClearColorValue | AppSettings::ClearColor              | Specifies which color the screen will be cleared with. |               |
| AppSettings.PrimitiveRestartEnabled | bool              | AppSettings::PrimitiveRestartEnabled | Toggle primitive restart index setting for pipeline.   |               |
| AppSettings.PolygonMode             | VkPolygonMode     | AppSettings::PolygonMode             | Specifies which polygon mode will be applied.          |               |
| AppSettings.LineWidth               | float             | AppSettings::LineWidth               | Specifies line width (if line mode has been selected). |               |


## Learning Objectives

- Changing some rasterization related values in pipeline
- Drawing lines or points instead of filled drawings
- Understanding the use of primitive restart index
- Adjusting line width in line drawing mode

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain

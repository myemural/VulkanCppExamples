# Drawing a Quad with Indices

**Code Name:** DrawingQuad

## Description

This example draws a quad to the screen with using index buffer in addition to vertex buffer.

## Screenshots / Recordings

![](/Docs/ExampleMedia/Fundamentals/Basics/DrawingQuad.png?raw=true)

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

| Parameter / Key        | Type              | Usage in Code           | Description                                            | Default Value |
|------------------------|-------------------|-------------------------|--------------------------------------------------------|---------------|
| AppSettings.ClearColor | VkClearColorValue | AppSettings::ClearColor | Specifies which color the screen will be cleared with. | Yes           |


## Learning Objectives

- Creating a index buffer and bind it via command
- Doing indexed drawing

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain

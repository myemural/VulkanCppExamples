# Drawing Multiple Triangles

**Code Name:** DrawingMultipleTriangles

## Description

This example draws 4 identical triangles to the screen to show how to draw something to the screen with using vertex buffer.

## Screenshots / Recordings

![](/Docs/ExampleMedia/Fundamentals/Basics/DrawingMultipleTriangles.png?raw=true)

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

- Creating a vertex buffer and fill it with data
- Binding vertex buffer with command
- Using vertex buffer in a shader with vertex attributes

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain

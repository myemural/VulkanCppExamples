# Drawing a Single Color Triangle

**Code Name:** DrawingSingleColorTriangle

## Description

This examples show hot to draw a single colored triangle to the screen to demonstrate pipeline and simple vertex and fragment shader usages.

## Screenshots / Recordings

![](/Docs/ExampleMedia/Fundamentals/Basics/DrawingSingleColorTriangle.png?raw=true)

## Controls

| Input | Action           |
|-------|------------------|
| Esc   | Close the window |

## Application Parameters

### Constants

| Parameter / Key                     | Type           | Usage in Code                        | Description                                                  | Default Value        |
|-------------------------------------|----------------|--------------------------------------|--------------------------------------------------------------|----------------------|
| AppConstants.MaxFramesInFlight      | std::uint32_t  | AppConstants::MaxFramesInFlight      | Maximum number of images to be processed in the render loop. | 2                    |
| AppConstants.VertexCount            | std::uint32_t  | AppConstants::VertexCount            | Specifies the vertex count that will be drawn.               | 3                    |
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

- Creating and managing shader modules
- Writing simple vertex and fragment shaders
- Creating a simple graphics pipeline
- Binding pipeline and drawing to framebuffers using it

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain

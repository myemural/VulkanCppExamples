# Drawing Triangles Directly with Mesh Shader

**Code Name:** TrianglesWithMeshShader

## Description

In this example, a maximum of 3 triangles are drawn on the screen using a mesh shader and the task/mesh pipeline.

## Output

![](/Docs/ExampleMedia/AdvancedShaderProgramming/MeshAndTaskShaders/TrianglesWithMeshShader.png?raw=true)

## Controls

| Input | Action           |
|-------|------------------|
| Esc   | Close the window |
| 1     | Draw 1 triangle  |
| 2     | Draw 2 triangles |
| 3     | Draw 3 triangles |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                      | Default Value |
|------------------------------|-------------------|-------------------------------|----------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen   |               |

## Learning Objectives

- How to implement and use mesh shaders in Vulkan
- Using mesh/task pipeline instead of conventional graphics pipeline
- Drawing triangles on the screen by providing data directly from the mesh shader itself

## Shader Status

| Shader Type | Status             | Notes                |
|-------------|--------------------|----------------------|
| GLSL        | :white_check_mark: |                      |
| HLSL        | :x:                | Will be implemented. |
| Slang       | :x:                | Will be implemented. |

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain
- VK_KHR_spirv_1_4
- VK_EXT_mesh_shader
- VK_KHR_shader_float_controls

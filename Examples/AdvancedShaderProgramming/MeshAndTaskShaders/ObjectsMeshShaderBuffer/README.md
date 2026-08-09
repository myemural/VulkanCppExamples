# Drawing 3D Objects with Mesh Shader by Reading Data from Buffers

**Code Name:** ObjectsMeshShaderBuffer

## Description

This example demonstrates how to draw 3D objects on the screen using a mesh shader, similar to a normal pipeline flow, by retrieving position, normal, and index data from external buffers.

## Output

![](/Docs/ExampleMedia/AdvancedShaderProgramming/MeshAndTaskShaders/ObjectsMeshShaderBuffer.png?raw=true)

## Controls

| Input   | Action                           |
|---------|----------------------------------|
| W/A/S/D | Move the camera                  |
| Mouse   | Look around with the camera      |
| Esc     | Close the window                 |

## Application Parameters

### Settings

| Parameter / Key               | Type              | Usage in Code                  | Description                    | Default Value |
|-------------------------------|-------------------|--------------------------------|--------------------------------|---------------|
| AppSettings.ClearColor        | VkClearColorValue | AppSettings::ClearColor        | Background color of the screen |               |
| AppSettings.MouseSensitivity  | float             | AppSettings::MouseSensitivity  | Mouse sensitivity value        |               |
| AppSettings.CameraSpeed       | float             | AppSettings::CameraSpeed       | Speed of the camera            |               |

## Learning Objectives

- Drawing 3D objects via mesh shader using external buffers

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

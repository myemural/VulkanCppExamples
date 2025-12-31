# Getting Vulkan and Device Info

**Code Name:** GetDeviceInfo

## Description

This example shows you how to list the GPUs in your system and write information about their Vulkan support and limits.

## Output

**Console Output**
```bash
*********************************************************
*Device Name: Intel(R) Iris(R) Xe Graphics              *
*********************************************************
Vulkan API Version: 1.3.212
Driver Version: 1656863
Vendor ID: 32902
Device ID: 18086
Device Type: Integrated GPU
*********************************************************
************ Device Limits ************
Max Image Dimension 2D: 16384
Max Uniform Buffer Range: 134217724
Max Vertex Input Attributes: 32
Max Vertex Input Bindings: 32
Max Push Constants Size: 256
************ Queue Families ************
Queue Family 0:
->Queue Count: 1
->Flags: Graphics Compute Transfer SparseBinding
Queue Family 1:
->Queue Count: 1
->Flags: Compute
*********************************************************
```

## Controls

None

## Application Parameters

None

## Learning Objectives

- Creating a Vulkan instance
- Getting physical device information
- Printing Vulkan capabilities of a device (GPU)

## Shader Status

| Shader Type | Status  | Notes |
|-------------|---------|-------|
| GLSL        | No Need |       |
| HLSL        | No Need |       |
| Slang       | No Need |       |

## Theoretical Background

None

## Extensions Used

### Instance

None

### Device

None

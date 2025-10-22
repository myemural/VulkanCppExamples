# Basics

**Code Name:** Basics

This subsection contains simply creating a window, getting device (GPU) info on your system and basic Vulkan implementations (like drawing triangle, using buffers etc.) examples.

The examples in this subsection, listed from easy to complex, are as follows:

1. [Creating a Window](/Examples/Fundamentals/Basics/CreatingWindow)
   - `CreatingWindow`
2. [Getting Vulkan and Device Info](/Examples/Fundamentals/Basics/GetDeviceInfo)
    - `GetDeviceInfo`
3. [Clear Screen with a Color](/Examples/Fundamentals/Basics/ClearScreenWithColor)
   - `ClearScreenWithColor`
4. [Drawing a Single Color Triangle](/Examples/Fundamentals/Basics/DrawingSingleColorTriangle)
   - `DrawingSingleColorTriangle`
5. [Drawing Multiple Triangles](/Examples/Fundamentals/Basics/DrawingMultipleTriangles)
   - `DrawingMultipleTriangles`
6. [Drawing Multicolor Triangles](/Examples/Fundamentals/Basics/DrawingMulticolorTriangles)
   - `DrawingMulticolorTriangles`
7. [Drawing a Quad with Indices](/Examples/Fundamentals/Basics/DrawingQuad)
   - `DrawingQuad`
8. [Drawing with Different Topologies](/Examples/Fundamentals/Basics/DrawingWithDifferentTopology)
   - `DrawingWithDifferentTopology`
9. [Using Staging Buffer](/Examples/Fundamentals/Basics/UsingStagingBuffer)
   - `UsingStagingBuffer`

## Architecture of the Subsection

The `VulkanApplication` classes used in the examples, except for the `CreatingWindow` and `GetDeviceInfo` examples, are derived from the `ApplicationBasics` class. The `ApplicationBasics` class includes common functions such as selecting a PhysicalDevice and creating a Surface, Device, Queue; creating SwapChain and Framebuffers with ImageViews taken from this SwapChain; creating CommandPool, synchronization objects, and RenderPass.
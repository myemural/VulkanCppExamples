# Advanced Shader Programming

This section contains Vulkan examples that related to advanced shader programming. This section includes examples of writing and using geometry, tessellation, and mesh/task shaders, in addition to the vertex and fragment shaders written so far. Examples of advanced compute shaders are also included.

## [Geometry Shaders](/Examples/AdvancedShaderProgramming/GeometryShaders)

- Learn how to use geometry shaders properly in Vulkan
- Understand the primitive generation mechanism of geometry shaders
- Learn how to use geometry shaders to expand triangles away from their original positions
- Explore techniques for decomposing and reassembling 3D models using geometry shaders
- Visualizing surface normal vectors with using geometry shaders
- Visualizing wireframe overlays using geometry shaders
- Understand how to use geometry shaders for efficient cubemap generation
- Learn about the benefits of single-pass cubemap rendering
- Understand how to use viewport arrays with geometry shaders
- Learn how to render multiple viewports with different lighting conditions
- Understanding how to implement billboarding from basic points using geometry shaders
- Understand how geometry shaders can be used for procedural generation
- Learn how to expand single points into complex geometry (grass blades)
- Apply random placement and transformation techniques for grass generation

## [Tessellation Shaders](/Examples/AdvancedShaderProgramming/TessellationShaders)

- Learning how to use tessellation shaders in Vulkan
- Understanding how tessellation control variables are working
- Implementing basic tessellation evaluation shader
- Understanding how triangle tessellation is working
- Learning how to implement displacement mapping with tessellation shaders
- Understanding the interaction between tessellation and displacement techniques
- Creating terrains with using heightmaps
- Understanding and applying quad tessellation in tessellation shaders
- Visualizing lines that passing through patch points in terrain
- Understanding how to implement cubic Bézier curves with using isolines tessellation
- Understanding how isoline tessellation is working
- Learning how to drag and drop any points with mouse in Vulkan applications
- Learning how to implement Bézier surfaces with tessellation shaders
- Learning how to manipulate control points for surface deformation
- Understanding how patch topology is working on more than 4 patch vertices
- Implementing curved PN triangles technique in Vulkan with using tessellation shaders
- Implementing dynamic LOD on tessellated terrains in Vulkan
- Change tessellation amount per patch according to camera distance in tessellation control shader
- Implementing basic water (or ocean) surface simulation with using Gerstner waves on tessellated plane object
- Implementing the water foam effect that appears when the wave height increases

## [Mesh and Task Shaders](/Examples/AdvancedShaderProgramming/MeshAndTaskShaders)

- How to implement and use mesh shaders in Vulkan
- Using mesh/task pipeline instead of conventional graphics pipeline
- Drawing triangles on the screen by providing data directly from the mesh shader itself
- Drawing 3D objects via mesh shader using external buffers
- Understanding meshlet-based rendering with mesh shaders
- Loading and optimizing glTF models with meshlets
- How to implement and use task shaders in Vulkan
- Implementing frustum culling with task shaders
- Using payload logic between mesh and task shaders
- Showing the LOD of the model based on camera distance using task shaders

## [Advanced Compute Shaders](/Examples/AdvancedShaderProgramming/AdvancedComputeShaders)

- Understanding how to enable device feature for shader subgroup operations in Vulkan
- Learning how to use subgroup ballot and vote operations in shaders
- Implementing Conway's Game of Life using compute shaders
- Learning how to implement ping-pong buffering
- Understanding how to implement frustum culling and occlusion culling using compute shaders
- Implementing Hierarchical Z-Buffer Occlusion Culling technique in Vulkan
- Understanding how to implement real-time N-body gravitational simulation using compute shaders
- Understanding how to implement wind volume simulation using compute shaders
- Learning how to apply updated vertex data from compute shader to geometry shader
- Learning how to draw debug box for wind volume and how to show tile-based wind vectors
- Understanding how to implement cloth simulation on GPU using compute shaders
- Learning how to apply updated vertex data from compute shader to vertex shader
- Understanding how to implement ocean simulation using FFT technique with compute shaders
- Understanding how to implement 2D Kármán vortex street example in Vulkan
- Understanding how to implement fluid simulation using Lattice Boltzmann Method (LBM) with compute shaders
- Observing 2D fluid dynamics with 4 debug display modes (velocity magnitude, vorticity, density, dye)
- Understanding how to implement 3D smoke simulation with Navier-Stokes technique using compute shaders
- Creating Bounding Volume Hierarchy (BVH) in Vulkan
- Implementing GPU-based real-time ray-tracing using compute shaders

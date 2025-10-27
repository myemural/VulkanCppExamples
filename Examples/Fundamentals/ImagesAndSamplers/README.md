# Images and Samplers

**Code Name:** ImagesAndSamplers

This subsection contains Vulkan's image, image view and sampler related examples.

The examples in this subsection, listed from easy to complex, are as follows:

1. [Textured Quad](/Examples/Fundamentals/ImagesAndSamplers/TexturedQuad)
   - `TexturedQuad`
2. [Combined Image Sampler](/Examples/Fundamentals/ImagesAndSamplers/CombinedImageSampler)
   - `CombinedImageSampler`
3. [Using Different Wrap and Filtering Modes](/Examples/Fundamentals/ImagesAndSamplers/WrapAndFilteringModes)
   - `WrapAndFilteringModes`
4. [Using Multiple Textures](/Examples/Fundamentals/ImagesAndSamplers/UsingMultipleTextures)
   - `UsingMultipleTextures`
5. [Drawing Transparent Texture to Quads](/Examples/Fundamentals/ImagesAndSamplers/SimpleBlending)
   - `SimpleBlending`
6. [Using Texture Atlases](/Examples/Fundamentals/ImagesAndSamplers/TextureAtlases)
   - `TextureAtlases`

## Architecture of the Subsection

The example in this subsection are using `ApplicationImagesAndSamplers` class. This class is almost same with the `ApplicationDescriptorSets` class. But in addition to `ApplicationDescriptorSets` class, `ApplicationImagesAndSamplers` is using two VulkanFramework classes:

- `common::vulkan_framework::DescriptorRegistry`
- `common::vulkan_framework::DescriptorUpdater`

`DescriptorRegistry` class creates descriptor set and descriptor set layout objects with given information structure. `DescriptorUpdater` class updates the resource binding of the descriptor sets. Creating and updating descriptor sets functionalities are associated with the `ApplicationImagesAndSamplers` class. So the examples that derived from this class can use these functionalities from their base classes.

In addition, `ApplicationImagesAndSamplers` class provides `ChangeImageLayout` method to change image layout from one to other easily.
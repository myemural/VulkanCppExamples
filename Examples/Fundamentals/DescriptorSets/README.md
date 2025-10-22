# Descriptor Sets

**Code Name:** DescriptorSets

This subsection contains Vulkan's descriptor set related examples.

The examples in this subsection, listed from easy to complex, are as follows:

1. [Changing Color of a Triangle with Uniform Buffer](/Examples/Fundamentals/DescriptorSets/ChangingColorWithUB)
   - `ChangingColorWithUB`
2. [Using Different UBs for Different Areas of the Screen](/Examples/Fundamentals/DescriptorSets/MultipleUniformBuffers)
   - `MultipleUniformBuffers`
3. [Rotating and Scaling a Square Constantly](/Examples/Fundamentals/DescriptorSets/Transformation2dWithUB)
   - `Transformation2dWithUB`
4. [Change Square Color with Keyboard Input](/Examples/Fundamentals/DescriptorSets/BasicPushConstants)
   - `BasicPushConstants`
5. [Multiple Transform with Descriptor Arrays](/Examples/Fundamentals/DescriptorSets/ArrayOfUB)
   - `ArrayOfUB`

## Architecture of the Subsection

The example in this subsection are using `ApplicationDescriptorSets` class. This class is almost same with the `ApplicationBasics` class. But in addition to `ApplicationBasics` class, `ApplicationDescriptorSets` is using two VulkanFramework classes:

- `common::vulkan_framework::ShaderResource`
- `common::vulkan_framework::BufferResource`

`ShaderResource` class creates ShaderModule objects with given information structure. `BufferResource` class crates Buffer objects and sets buffer with the raw data. Creating ShaderModule, creating Buffer and setting Buffer functionalities are associated with the `ApplicationDescriptorSets` class. So the examples that derived from this class can use these functionalities from their base classes.
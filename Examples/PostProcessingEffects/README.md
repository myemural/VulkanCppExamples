# Post-Processing Effects

This section contains Vulkan examples that related to post-processing effects. This section includes examples of implementing various post-processing techniques such as ambient occlusion, bloom, motion blur, depth of field and more.

## [Ambient Occlusion](/Examples/PostProcessingEffects/AmbientOcclusion)

- Understanding how to implement Screen-Space Ambient Occlusion (SSAO)
- Implementing SSAO and SSAO Blur passes in Vulkan
- Understanding how to implement Horizon-Based Ambient Occlusion (HBAO)
- Implementing HBAO and HBAO Blur passes in Vulkan
- Understanding how to implement Ground-Truth Ambient Occlusion (GTAO)
- Implementing GTAO and GTAO Blur passes in Vulkan
- 
## [Color Processing and Tone Mapping](/Examples/PostProcessingEffects/ColorProcessingToneMapping)

- Understanding how to implement grayscale post-processing color filter effect
- Understanding how to implement invert color post-processing color filter effect
- Understanding how to implement sepia post-processing color filter effect
- Understanding how to implement duotone post-processing color filter effect
- Understanding how to implement channel swap post-processing color filter effect
- Understanding how to implement brightness post-processing color correction effect
- Understanding how to implement contrast color post-processing color correction effect
- Understanding how to implement saturation post-processing color correction effect
- Understanding how to implement hue shift post-processing color correction effect
- Understanding how to implement Reinhard tone mapping algorithm
- Understanding how to implement ACES tone mapping algorithm
- Understanding how to implement Uchimura tone mapping algorithm
- Understanding how to implement AgX tone mapping algorithm
- Understanding how to implement Parametric Filmic tone mapping algorithm
- Understanding how to implement white balance and color temperature post-processing effect on HDR scene color
- Understanding how to implement scene-referred lift/gamma/gain applied to the linear HDR scene color
- Generating 3D LUT image using a compute shader
- Applying color grading post-processing effect using generated 3D LUT image

## [Exposure and Adaptation](/Examples/PostProcessingEffects/ExposureAndAdaptation)

- Auto exposure value calculation with log-average luminance of the HDR scene
- Histogram-based auto exposure calculation
- Applying eye adaptation on histogram-based auto exposure
- Observing instantaneous light changes on eye adaptation technique
- Applying local exposure technique using a blend of a bilateral grid and a blurred luminance
- Integrating eye adaptation on local exposure

## [Lens and Camera Effects](/Examples/PostProcessingEffects/LensAndCameraEffects)

- Understanding how to implement film grain post-processing effect
- Adjusting intensity, size and luminance response values of the film grain effect
- Turning on/off colored film grain effect
- Understanding how to implement vignette post-processing effect
- Adjusting intensity, smoothness and roundness values of the vignette effect
- Applying different kind of vignette theme presets
- Understanding how to implement lens distortion (barrel, pincushion, fisheye) post-processing effects
- Adjusting strength and zoom values of the distortion effects
- Enabling or disabling auto-fit feature of the barrel distortion effect
- Understanding how to implement chromatic aberration post-processing effect
- Adjusting intensity and falloff values of the chromatic aberration effect
- Understanding how to implement procedural lens flare post-processing effect
- Enabling or disabling glow, starburst, streak, ghosts and halo components of the lens flare effect
- Adjusting intensity value of the lens flare effect

## [Bloom and Glow](/Examples/PostProcessingEffects/BloomAndGlow)

- Understanding how to implement a thresholded bloom post-processing effect on HDR screen color
- Extracting bright areas with a soft-l-knee threshold into a half-resolution image
- Implementing a separable gaussian blur with linear sampling and ping-pong render targets
- Understanding how to implement a physically based (threshold-less) bloom effect with a mip chain
- Downsampling with the 13-tap filter and suppressing fireflies with Karis average
- Upsampling with a 3x3 tent filter and accumulating the mip chain with additive blending
- Understanding how to apply a lens dirt texture to the bloom effect
- Understanding how to create an anamorphic (horizontally stretched) bloom by scaling the upsample filter

## [Analog Display Effects](/Examples/PostProcessingEffects/AnalogDisplayEffects)

TBD

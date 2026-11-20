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

- Understanding how to simulate a CRT display with a low-resolution virtual image
- Drawing scanlines with a brightness dependent gaussian beam profile without losing the average brightness
- Implementing aperture grille, slot mask and shadow mask phosphor patterns
- Understanding how to simulate the curved tube of a CRT with rounded corners
- Implementing halation with a separate gaussian blur of the low-res virtual image
- Implementing interlacing with a persistent render target, blending and phosphor persistence
- Understanding how to simulate the limited luma and chroma bandwidth of a VHS tape in the YIQ color space
- Implementing time dependent line displacements (wobble, jitter, tracking error, head switching)
- Generating tape noise and dropouts procedurally with has functions

## [Blur and Motion Effects](/Examples/PostProcessingEffects/BlurAndMotionEffects)

- Understanding how separable box and gaussian blurs work and how linear sampling halves the texture fetches
- Understanding how Kawase blur approximates a wide gaussian with a few cheap iterations
- Understanding how Dual Kawase blur uses a downsample/upsample chain to get very wide blurs
- Calculating the circle of confusion (CoC) with the thin lens model from focal length, f-stop and focus distance
- Separating the near and far fields with a signed CoC and gathering them as different layers
- Using depth-aware weighting to prevent bleeding of the in-focus objects onto the blurred background
- Spreading the near field over the in-focus areas with scatter-as-gatter and calculating its coverage
- Running the expensive gather pass in half resolution and mixing the result with the full-res image
- Understanding how bokeh shapes are formed by the aperture of a real lens
- Separating the energy of bright highlights from the gathered image without losing or duplicating energy
- Scattering sprites with an instanced draw call without vertex buffers and culling them in the vertex shader
- Using additive blending to accumulate the sprites and occluding far field sprites with the CoC
- Reconstructing the screen space velocity of the camera motion from the depth (view-space position) and the previous view-projection matrix
- Averaging the color along the velocity and hiding the banding with a per-pixel jitter
- Scaling the blur length with the shutter speed and the frame time
- Writing a velocity buffer in the geometry pass with the current and the previous frame's matrices
- Understanding the TileMax and NeighborMax passes and why the blur needs the velocities of the neighbor tiles
- Implementing the reconstruction filter (McGuire et al. 2012) with soft depth comparisons, cone and cylinder weights

## [Stylization and Image Effects](/Examples/PostProcessingEffects/StylizationAndImageEffects)

- Implementing pixelation y averaging the colors of screen-space blocks
- Understanding why the quantization (posterization) is done after tone mapping and in the display (sRGB) space
- Constructing a Bayer 8x8 matrix with bit operations and using it for ordered dithering
- Hiding the banding of the posterization with ordered dithering
- Understanding how the Sobel operator finds the edges with the gradient of an image
- Detecting silhouettes with linear depth and creases with normals from the G-Buffer
- Using relative depth differences and a grazing angle correction to avoid false edges
- Implementing cel shading in a deferred light pass
- Quantizing the brightness of the total diffuse light with a toon ramp while keeping the light color
- Adding hard-edged specular highlights and rim light for the toon look
- Understanding the Kuwahara filter as an edge-preserving smoothing filter
- Removing the blocky artifacts of the basic filter with smoothly weighted sectors (polynomial weights)
- Finding the local orientation and anisotropy with the structure tensor

## [Anti-Aliasing Techniques](/Examples/PostProcessingEffects/AntiAliasingTechniques)

TBD

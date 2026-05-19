set(SUBCATEGORY_NAME TexturedPbr)

add_subdirectory(Base)
include_directories(Base)

add_subdirectory(RoughnessMapPbr)
add_subdirectory(MetallicMapPbr)
add_subdirectory(NormalMapPbr)
add_subdirectory(AmbientOcclusionPbr)
add_subdirectory(EmissiveMapPbr)
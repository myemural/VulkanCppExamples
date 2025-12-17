set(SUBCATEGORY_NAME LightSources)

add_subdirectory(Base)
include_directories(Base)

add_subdirectory(DirectionalLight)
add_subdirectory(PointLight)
add_subdirectory(Spotlight)
add_subdirectory(Flashlight)
OPTIX_add_sample_executable( PistonOptix
  src/main.cpp

  inc/Application.h
  src/Application.cpp

  src/Box.cpp
  src/Plane.cpp
  src/Sphere.cpp
  src/Torus.cpp

  inc/PinholeCamera.h
  src/PinholeCamera.cpp

  inc/Timer.h
  src/Timer.cpp

  inc/MyAssert.h

  inc/CudaUtils/State.h

  shaders/app_config.h
  shaders/material_parameter.h
  shaders/per_ray_data.h
  shaders/random_number_generators.h
  shaders/rt_assert.h
  shaders/rt_function.h
  shaders/shader_common.h
  shaders/vertex_attributes.h

  shaders/boundingbox_triangle_indexed.cu
  shaders/intersection_triangle_indexed.cu
  
  shaders/raygeneration.cu
  shaders/exception.cu
  shaders/miss.cu

  shaders/closesthit.cu
  shaders/lambert.cu
  shaders/PhongModified.cu

  
)    

include_directories(
  "."
)

cmake_minimum_required( VERSION 2.8.6 )

project(BloodFlowPhantomGeneration)

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#Directories to include
include_directories(Common)
include_directories(DistanceTransform)
include_directories(PhantomGenerator)

#Executables 
add_executable( SeparateVesselRegions
   Common/ArgumentsReader.h
   Common/ImageManagement.h
   Common/ImageManagement.cpp
   DistanceTransform/VesselRadii.h
   DistanceTransform/VesselRadii.cpp    
   PhantomGenerator/SeparateVesselRegions.h
   PhantomGenerator/SeparateVesselRegions.cpp
)

add_executable( GenerateParametersAndImages
   Common/ArgumentsReader.h
   Common/ImageManagement.h
   Common/ImageManagement.cpp
   DistanceTransform/VesselRadii.h
   DistanceTransform/VesselRadii.cpp      
   PhantomGenerator/GenerateParametersAndImages.h
   PhantomGenerator/GenerateParametersAndImages.cpp
)


add_executable( ComputeSkeleton
   Common/ArgumentsReader.h
   Common/ImageManagement.h
   Common/ImageManagement.cpp
   PhantomGenerator/ComputeSkeleton.h
   PhantomGenerator/ComputeSkeleton.cpp
)

#Link libraries
target_link_libraries(SeparateVesselRegions ${ITK_LIBRARIES})
target_link_libraries(GenerateParametersAndImages ${ITK_LIBRARIES})
target_link_libraries(ComputeSkeleton ${ITK_LIBRARIES})

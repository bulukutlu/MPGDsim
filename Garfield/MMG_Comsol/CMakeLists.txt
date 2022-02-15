# ########################################################################################
# Use the code in this section for standalone projects
cmake_minimum_required(VERSION 3.9 FATAL_ERROR)
project(MMGComsol)
if(NOT TARGET Garfield::Garfield)
  find_package(Garfield REQUIRED)
endif()
# ########################################################################################

# ---Build executable------------------------------------------------------------
add_executable(mmg MMG.C)
add_executable(mmg_single MMG_single)
target_link_libraries(mmg Garfield::Garfield)
target_link_libraries(mmg_single Garfield::Garfield)

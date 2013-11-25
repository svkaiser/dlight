##-----------------------------------------------------------------------------
##
## Source Level CMake file.
##
##-----------------------------------------------------------------------------

include_directories(${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/kexlib ${CMAKE_SOURCE_DIR}/src/kexlib/math)

add_executable(dlight
main.cpp
surfaces.cpp
wad.cpp
kexlib/binFile.cpp
kexlib/kstring.cpp
kexlib/memHeap.cpp
kexlib/math/angle.cpp
kexlib/math/bounds.cpp
kexlib/math/mathlib.cpp
kexlib/math/matrix.cpp
kexlib/math/plane.cpp
kexlib/math/pluecker.cpp
kexlib/math/quaternion.cpp
kexlib/math/random.cpp
kexlib/math/vector.cpp
)

target_link_libraries(dlight m)


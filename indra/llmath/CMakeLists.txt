# -*- cmake -*-

project(llmath)

include(00-Common)
include(LLCommon)
include(bugsplat)
include(Boost)

set(llmath_SOURCE_FILES
    llbbox.cpp
    llbboxlocal.cpp
    llcalc.cpp
    llcalcparser.cpp
    llcamera.cpp
    llcoordframe.cpp
    llline.cpp
    llmatrix3a.cpp
    llmatrix4a.cpp
    llmodularmath.cpp
    lloctree.cpp
    llperlin.cpp
    llquaternion.cpp
    llrigginginfo.cpp
    llrect.cpp
    llsphere.cpp
    llvector4a.cpp
    llvolume.cpp
    llvolumemgr.cpp
    llvolumeoctree.cpp
    llsdutil_math.cpp
    m3math.cpp
    m4math.cpp
    raytrace.cpp
    v2math.cpp
    v3color.cpp
    v3dmath.cpp
    v3math.cpp
    v4color.cpp
    v4coloru.cpp
    v4math.cpp
    xform.cpp
    )

set(llmath_HEADER_FILES
    CMakeLists.txt

    camera.h
    coordframe.h
    llbbox.h
    llbboxlocal.h
    llcalc.h
    llcalcparser.h
    llcamera.h
    llcoord.h
    llcoordframe.h
    llinterp.h
    llline.h
    llmath.h
    llmatrix3a.h
    llmatrix3a.inl
    llmatrix4a.h
    llmodularmath.h
    lloctree.h
    llperlin.h
    llplane.h
    llquantize.h
    llquaternion.h
    llquaternion2.h
    llquaternion2.inl
    llrect.h
    llrigginginfo.h
    llsimdmath.h
    llsimdtypes.h
    llsimdtypes.inl
    llsphere.h
    lltreenode.h
    llvector4a.h
    llvector4a.inl
    llvector4logical.h
    llvolume.h
    llvolumemgr.h
    llvolumeoctree.h
    llsdutil_math.h
    m3math.h
    m4math.h
    raytrace.h
    v2math.h
    v3color.h
    v3colorutil.h
    v3dmath.h
    v3math.h
    v4color.h
    v4coloru.h
    v4math.h
    xform.h
    )

set( llmath_SOURCE_FILES ${llmath_SOURCE_FILES} nd/ndoctreelog.cpp )
set( llmath_HEADER_FILES ${llmath_HEADER_FILES} nd/ndoctreelog.h )

list(APPEND llmath_SOURCE_FILES ${llmath_HEADER_FILES})

add_library (llmath ${llmath_SOURCE_FILES})

target_link_libraries(llmath llcommon llmeshoptimizer)
target_include_directories( llmath  INTERFACE   ${CMAKE_CURRENT_SOURCE_DIR})

# <MG:Aleric> mold requires this: llmath is a dependency of llplugin (and llimage), which is a dependency of media_plugin_base, which is a dependency of
# media_plugin_cef, media_plugin_libvlc, media_plugin_gstreamer010 and media_plugin_gstreamer10 that all are shared libraries.
set_property(TARGET llmath PROPERTY POSITION_INDEPENDENT_CODE ON)

# Add tests
if (LL_TESTS)
  include(LLAddBuildTest)
  # UNIT TESTS
  SET(llmath_TEST_SOURCE_FILES
    llbboxlocal.cpp
    llmodularmath.cpp
    llrect.cpp
    v2math.cpp
    v3color.cpp
    v4color.cpp
    v4coloru.cpp
    )

  LL_ADD_PROJECT_UNIT_TESTS(llmath "${llmath_TEST_SOURCE_FILES}")

  # INTEGRATION TESTS
  set(test_libs llmath llcommon)
  # TODO: Some of these need refactoring to be proper Unit tests rather than Integration tests.
  LL_ADD_INTEGRATION_TEST(alignment "" "${test_libs}")
  LL_ADD_INTEGRATION_TEST(llbbox llbbox.cpp "${test_libs}")
  LL_ADD_INTEGRATION_TEST(llquaternion llquaternion.cpp "${test_libs}")
  LL_ADD_INTEGRATION_TEST(mathmisc "" "${test_libs}")
  LL_ADD_INTEGRATION_TEST(m3math "" "${test_libs}")
  LL_ADD_INTEGRATION_TEST(v3dmath v3dmath.cpp "${test_libs}")
  LL_ADD_INTEGRATION_TEST(v3math v3math.cpp "${test_libs}")
  LL_ADD_INTEGRATION_TEST(v4math v4math.cpp "${test_libs}")
  LL_ADD_INTEGRATION_TEST(xform xform.cpp "${test_libs}")
endif (LL_TESTS)

#
# Build NvParameterized
#

SET(GW_DEPS_ROOT $ENV{GW_DEPS_ROOT})
FIND_PACKAGE(PxShared REQUIRED)

SET(APEX_MODULE_DIR ${PROJECT_SOURCE_DIR}/../../../module)

SET(NVP_SOURCE_DIR ${PROJECT_SOURCE_DIR}/../../../NvParameterized)
#SET(AM_SOURCE_DIR ${APEX_MODULE_DIR}/{{TARGET_MODULE_DIR}})

# Use generator expressions to set config specific preprocessor definitions
SET(NVPARAMETERIZED_COMPILE_DEFS 
	# Common to all configurations
	${APEX_LINUX_COMPILE_DEFS};_LIB;PX_PHYSX_STATIC_LIB;
	__Linux__;Linux;PX_FOUNDATION_DLL=0;
)

if(${CMAKE_BUILD_TYPE_LOWERCASE} STREQUAL "debug")
	LIST(APPEND NVPARAMETERIZED_COMPILE_DEFS
		${APEX_LINUX_DEBUG_COMPILE_DEFS};PX_PHYSX_DLL_NAME_POSTFIX=DEBUG;
	)
elseif(${CMAKE_BUILD_TYPE_LOWERCASE} STREQUAL "checked")
	LIST(APPEND NVPARAMETERIZED_COMPILE_DEFS
		${APEX_LINUX_CHECKED_COMPILE_DEFS};PX_PHYSX_DLL_NAME_POSTFIX=CHECKED;
	)
elseif(${CMAKE_BUILD_TYPE_LOWERCASE} STREQUAL "profile")
	LIST(APPEND NVPARAMETERIZED_COMPILE_DEFS
		${APEX_LINUX_PROFILE_COMPILE_DEFS};PX_PHYSX_DLL_NAME_POSTFIX=PROFILE;
	)
elseif(${CMAKE_BUILD_TYPE_LOWERCASE} STREQUAL release)
	LIST(APPEND NVPARAMETERIZED_COMPILE_DEFS
		${APEX_LINUX_RELEASE_COMPILE_DEFS}
	)
else(${CMAKE_BUILD_TYPE_LOWERCASE} STREQUAL "debug")
	MESSAGE(FATAL_ERROR "Unknown configuration ${CMAKE_BUILD_TYPE}")
endif(${CMAKE_BUILD_TYPE_LOWERCASE} STREQUAL "debug")


# include common NvParameterized.cmake
INCLUDE(../common/NvParameterized.cmake)

# enable -fPIC so we can link static libs with the editor
SET_TARGET_PROPERTIES(NvParameterized PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

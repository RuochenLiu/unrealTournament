/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


// This file was generated by NxParameterized/scripts/GenParameterized.pl
// Created: 2015.01.18 19:26:28

#ifndef HEADER_ClothingActorParam_0p12_h
#define HEADER_ClothingActorParam_0p12_h

#include "NxParametersTypes.h"

#ifndef NX_PARAMETERIZED_ONLY_LAYOUTS
#include "NxParameterized.h"
#include "NxParameters.h"
#include "NxParameterizedTraits.h"
#include "NxTraitsInternal.h"
#endif

namespace physx
{
namespace apex
{

#pragma warning(push)
#pragma warning(disable: 4324) // structure was padded due to __declspec(align())

namespace ClothingActorParam_0p12NS
{

struct GroupsMask_Type;
struct LODWeights_Type;
struct ClothDescTemplate_Type;
struct ShapeDescFlags_Type;
struct ShapeDescTemplate_Type;
struct ActorDescTemplate_Type;
struct WindParameters_Type;
struct MaxDistanceScale_Type;
struct ClothingActorFlags_Type;

struct MAT44_DynamicArray1D_Type
{
	physx::PxMat44* buf;
	bool isAllocated;
	physx::PxI32 elementSize;
	physx::PxI32 arraySizes[1];
};

struct VEC3_DynamicArray1D_Type
{
	physx::PxVec3* buf;
	bool isAllocated;
	physx::PxI32 elementSize;
	physx::PxI32 arraySizes[1];
};

struct STRING_DynamicArray1D_Type
{
	NxParameterized::DummyStringStruct* buf;
	bool isAllocated;
	physx::PxI32 elementSize;
	physx::PxI32 arraySizes[1];
};

struct ActorDescTemplate_Type
{
	physx::PxU64 userData;
	physx::PxU64 name;
};
struct GroupsMask_Type
{
	physx::PxU32 bits0;
	physx::PxU32 bits1;
	physx::PxU32 bits2;
	physx::PxU32 bits3;
};
struct ClothDescTemplate_Type
{
	physx::PxF32 collisionResponseCoefficient;
	physx::PxU16 collisionGroup;
	GroupsMask_Type groupsMask;
	physx::PxBounds3 validBounds;
	physx::PxU64 compartment;
	physx::PxU64 userData;
};
struct MaxDistanceScale_Type
{
	physx::PxF32 Scale;
	bool Multipliable;
};
struct ClothingActorFlags_Type
{
	bool ParallelCpuSkinning;
	bool RecomputeNormals;
	bool Visualize;
	bool CorrectSimulationNormals;
};
struct ShapeDescFlags_Type
{
	bool NX_SF_VISUALIZATION;
	bool NX_SF_DISABLE_COLLISION;
	bool NX_SF_DISABLE_RAYCASTING;
	bool NX_SF_DYNAMIC_DYNAMIC_CCD;
	bool NX_SF_DISABLE_SCENE_QUERIES;
};
struct ShapeDescTemplate_Type
{
	ShapeDescFlags_Type flags;
	physx::PxU16 collisionGroup;
	GroupsMask_Type groupsMask;
	physx::PxU16 materialIndex;
	physx::PxU64 userData;
	physx::PxU64 name;
};
struct WindParameters_Type
{
	physx::PxVec3 Velocity;
	physx::PxF32 Adaption;
};
struct LODWeights_Type
{
	physx::PxF32 maxDistance;
	physx::PxF32 distanceWeight;
	physx::PxF32 bias;
	physx::PxF32 benefitsBias;
};

struct ParametersStruct
{

	physx::PxMat44 globalPose;
	bool useHardwareCloth;
	ClothingActorFlags_Type flags;
	bool fallbackSkinning;
	bool slowStart;
	bool useInternalBoneOrder;
	bool updateStateWithGlobalMatrices;
	physx::PxU32 uvChannelForTangentUpdate;
	physx::PxF32 maxDistanceBlendTime;
	physx::PxU32 clothingMaterialIndex;
	LODWeights_Type lodWeights;
	WindParameters_Type windParams;
	MaxDistanceScale_Type maxDistanceScale;
	physx::PxU64 userData;
	MAT44_DynamicArray1D_Type boneMatrices;
	ClothDescTemplate_Type clothDescTemplate;
	ShapeDescTemplate_Type shapeDescTemplate;
	ActorDescTemplate_Type actorDescTemplate;
	physx::PxF32 actorScale;
	NxParameterized::Interface* runtimeCooked;
	VEC3_DynamicArray1D_Type morphDisplacements;
	VEC3_DynamicArray1D_Type morphPhysicalMeshNewPositions;
	VEC3_DynamicArray1D_Type morphGraphicalMeshNewPositions;
	bool allowAdaptiveTargetFrequency;
	bool useVelocityClamping;
	physx::PxBounds3 vertexVelocityClamp;
	physx::PxF32 pressure;
	bool multiplyGlobalPoseIntoBones;
	STRING_DynamicArray1D_Type overrideMaterialNames;
	NxParameterized::DummyStringStruct simulationBackend;
	bool freezeByLOD;

};

static const physx::PxU32 checksum[] = { 0xd2c0383d, 0xd509d3bb, 0x5705bfa6, 0xa9b64c16, };

} // namespace ClothingActorParam_0p12NS

#ifndef NX_PARAMETERIZED_ONLY_LAYOUTS
class ClothingActorParam_0p12 : public NxParameterized::NxParameters, public ClothingActorParam_0p12NS::ParametersStruct
{
public:
	ClothingActorParam_0p12(NxParameterized::Traits* traits, void* buf = 0, PxI32* refCount = 0);

	virtual ~ClothingActorParam_0p12();

	virtual void destroy();

	static const char* staticClassName(void)
	{
		return("ClothingActorParam");
	}

	const char* className(void) const
	{
		return(staticClassName());
	}

	static const physx::PxU32 ClassVersion = ((physx::PxU32)0 << 16) + (physx::PxU32)12;

	static physx::PxU32 staticVersion(void)
	{
		return ClassVersion;
	}

	physx::PxU32 version(void) const
	{
		return(staticVersion());
	}

	static const physx::PxU32 ClassAlignment = 8;

	static const physx::PxU32* staticChecksum(physx::PxU32& bits)
	{
		bits = 8 * sizeof(ClothingActorParam_0p12NS::checksum);
		return ClothingActorParam_0p12NS::checksum;
	}

	static void freeParameterDefinitionTable(NxParameterized::Traits* traits);

	const physx::PxU32* checksum(physx::PxU32& bits) const
	{
		return staticChecksum(bits);
	}

	const ClothingActorParam_0p12NS::ParametersStruct& parameters(void) const
	{
		ClothingActorParam_0p12* tmpThis = const_cast<ClothingActorParam_0p12*>(this);
		return *(static_cast<ClothingActorParam_0p12NS::ParametersStruct*>(tmpThis));
	}

	ClothingActorParam_0p12NS::ParametersStruct& parameters(void)
	{
		return *(static_cast<ClothingActorParam_0p12NS::ParametersStruct*>(this));
	}

	virtual NxParameterized::ErrorType getParameterHandle(const char* long_name, NxParameterized::Handle& handle) const;
	virtual NxParameterized::ErrorType getParameterHandle(const char* long_name, NxParameterized::Handle& handle);

	void initDefaults(void);

protected:

	virtual const NxParameterized::DefinitionImpl* getParameterDefinitionTree(void);
	virtual const NxParameterized::DefinitionImpl* getParameterDefinitionTree(void) const;


	virtual void getVarPtr(const NxParameterized::Handle& handle, void*& ptr, size_t& offset) const;

private:

	void buildTree(void);
	void initDynamicArrays(void);
	void initStrings(void);
	void initReferences(void);
	void freeDynamicArrays(void);
	void freeStrings(void);
	void freeReferences(void);

	static bool mBuiltFlag;
	static NxParameterized::MutexType mBuiltFlagMutex;
};

class ClothingActorParam_0p12Factory : public NxParameterized::Factory
{
	static const char* const vptr;

public:
	virtual NxParameterized::Interface* create(NxParameterized::Traits* paramTraits)
	{
		// placement new on this class using mParameterizedTraits

		void* newPtr = paramTraits->alloc(sizeof(ClothingActorParam_0p12), ClothingActorParam_0p12::ClassAlignment);
		if (!NxParameterized::IsAligned(newPtr, ClothingActorParam_0p12::ClassAlignment))
		{
			NX_PARAM_TRAITS_WARNING(paramTraits, "Unaligned memory allocation for class ClothingActorParam_0p12");
			paramTraits->free(newPtr);
			return 0;
		}

		memset(newPtr, 0, sizeof(ClothingActorParam_0p12)); // always initialize memory allocated to zero for default values
		return NX_PARAM_PLACEMENT_NEW(newPtr, ClothingActorParam_0p12)(paramTraits);
	}

	virtual NxParameterized::Interface* finish(NxParameterized::Traits* paramTraits, void* bufObj, void* bufStart, physx::PxI32* refCount)
	{
		if (!NxParameterized::IsAligned(bufObj, ClothingActorParam_0p12::ClassAlignment)
		        || !NxParameterized::IsAligned(bufStart, ClothingActorParam_0p12::ClassAlignment))
		{
			NX_PARAM_TRAITS_WARNING(paramTraits, "Unaligned memory allocation for class ClothingActorParam_0p12");
			return 0;
		}

		// Init NxParameters-part
		// We used to call empty constructor of ClothingActorParam_0p12 here
		// but it may call default constructors of members and spoil the data
		NX_PARAM_PLACEMENT_NEW(bufObj, NxParameterized::NxParameters)(paramTraits, bufStart, refCount);

		// Init vtable (everything else is already initialized)
		*(const char**)bufObj = vptr;

		return (ClothingActorParam_0p12*)bufObj;
	}

	virtual const char* getClassName()
	{
		return (ClothingActorParam_0p12::staticClassName());
	}

	virtual physx::PxU32 getVersion()
	{
		return (ClothingActorParam_0p12::staticVersion());
	}

	virtual physx::PxU32 getAlignment()
	{
		return (ClothingActorParam_0p12::ClassAlignment);
	}

	virtual const physx::PxU32* getChecksum(physx::PxU32& bits)
	{
		return (ClothingActorParam_0p12::staticChecksum(bits));
	}
};
#endif // NX_PARAMETERIZED_ONLY_LAYOUTS

} // namespace apex
} // namespace physx

#pragma warning(pop)

#endif

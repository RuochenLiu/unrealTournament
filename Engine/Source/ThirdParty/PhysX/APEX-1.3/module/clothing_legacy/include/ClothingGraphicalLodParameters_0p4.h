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

#ifndef HEADER_ClothingGraphicalLodParameters_0p4_h
#define HEADER_ClothingGraphicalLodParameters_0p4_h

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

namespace ClothingGraphicalLodParameters_0p4NS
{

struct SkinClothMapB_Type;
struct SkinClothMapC_Type;
struct SkinClothMapD_Type;
struct TetraLink_Type;
struct PhysicsSubmeshPartitioning_Type;

struct STRING_DynamicArray1D_Type
{
	NxParameterized::DummyStringStruct* buf;
	bool isAllocated;
	physx::PxI32 elementSize;
	physx::PxI32 arraySizes[1];
};

struct U32_DynamicArray1D_Type
{
	physx::PxU32* buf;
	bool isAllocated;
	physx::PxI32 elementSize;
	physx::PxI32 arraySizes[1];
};

struct SkinClothMapB_DynamicArray1D_Type
{
	SkinClothMapB_Type* buf;
	bool isAllocated;
	physx::PxI32 elementSize;
	physx::PxI32 arraySizes[1];
};

struct SkinClothMapD_DynamicArray1D_Type
{
	SkinClothMapD_Type* buf;
	bool isAllocated;
	physx::PxI32 elementSize;
	physx::PxI32 arraySizes[1];
};

struct TetraLink_DynamicArray1D_Type
{
	TetraLink_Type* buf;
	bool isAllocated;
	physx::PxI32 elementSize;
	physx::PxI32 arraySizes[1];
};

struct PhysicsSubmeshPartitioning_DynamicArray1D_Type
{
	PhysicsSubmeshPartitioning_Type* buf;
	bool isAllocated;
	physx::PxI32 elementSize;
	physx::PxI32 arraySizes[1];
};

struct SkinClothMapC_Type
{
	physx::PxVec3 vertexBary;
	physx::PxU32 faceIndex0;
	physx::PxVec3 normalBary;
	physx::PxU32 vertexIndexPlusOffset;
};
struct PhysicsSubmeshPartitioning_Type
{
	physx::PxU32 graphicalSubmesh;
	physx::PxU32 physicalSubmesh;
	physx::PxU32 numSimulatedVertices;
	physx::PxU32 numSimulatedVerticesAdditional;
	physx::PxU32 numSimulatedIndices;
};
struct SkinClothMapD_Type
{
	physx::PxVec3 vertexBary;
	physx::PxU32 vertexIndex0;
	physx::PxVec3 normalBary;
	physx::PxU32 vertexIndex1;
	physx::PxVec3 tangentBary;
	physx::PxU32 vertexIndex2;
	physx::PxU32 vertexIndexPlusOffset;
};
struct SkinClothMapB_Type
{
	physx::PxVec3 vtxTetraBary;
	physx::PxU32 vertexIndexPlusOffset;
	physx::PxVec3 nrmTetraBary;
	physx::PxU32 faceIndex0;
	physx::PxU32 tetraIndex;
	physx::PxU32 submeshIndex;
};
struct TetraLink_Type
{
	physx::PxVec3 vertexBary;
	physx::PxU32 tetraIndex0;
	physx::PxVec3 normalBary;
	physx::PxU32 _dummyForAlignment;
};

struct ParametersStruct
{

	STRING_DynamicArray1D_Type platforms;
	physx::PxU32 lod;
	physx::PxU32 physicalMeshId;
	NxParameterized::Interface* renderMeshAsset;
	void* renderMeshAssetPointer;
	U32_DynamicArray1D_Type immediateClothMap;
	SkinClothMapB_DynamicArray1D_Type skinClothMapB;
	SkinClothMapD_DynamicArray1D_Type skinClothMap;
	physx::PxF32 skinClothMapThickness;
	physx::PxF32 skinClothMapOffset;
	TetraLink_DynamicArray1D_Type tetraMap;
	physx::PxU32 renderMeshAssetSorting;
	PhysicsSubmeshPartitioning_DynamicArray1D_Type physicsSubmeshPartitioning;

};

static const physx::PxU32 checksum[] = { 0x8cf41087, 0x1ec73f2e, 0x602ad91d, 0xdbec5061, };

} // namespace ClothingGraphicalLodParameters_0p4NS

#ifndef NX_PARAMETERIZED_ONLY_LAYOUTS
class ClothingGraphicalLodParameters_0p4 : public NxParameterized::NxParameters, public ClothingGraphicalLodParameters_0p4NS::ParametersStruct
{
public:
	ClothingGraphicalLodParameters_0p4(NxParameterized::Traits* traits, void* buf = 0, PxI32* refCount = 0);

	virtual ~ClothingGraphicalLodParameters_0p4();

	virtual void destroy();

	static const char* staticClassName(void)
	{
		return("ClothingGraphicalLodParameters");
	}

	const char* className(void) const
	{
		return(staticClassName());
	}

	static const physx::PxU32 ClassVersion = ((physx::PxU32)0 << 16) + (physx::PxU32)4;

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
		bits = 8 * sizeof(ClothingGraphicalLodParameters_0p4NS::checksum);
		return ClothingGraphicalLodParameters_0p4NS::checksum;
	}

	static void freeParameterDefinitionTable(NxParameterized::Traits* traits);

	const physx::PxU32* checksum(physx::PxU32& bits) const
	{
		return staticChecksum(bits);
	}

	const ClothingGraphicalLodParameters_0p4NS::ParametersStruct& parameters(void) const
	{
		ClothingGraphicalLodParameters_0p4* tmpThis = const_cast<ClothingGraphicalLodParameters_0p4*>(this);
		return *(static_cast<ClothingGraphicalLodParameters_0p4NS::ParametersStruct*>(tmpThis));
	}

	ClothingGraphicalLodParameters_0p4NS::ParametersStruct& parameters(void)
	{
		return *(static_cast<ClothingGraphicalLodParameters_0p4NS::ParametersStruct*>(this));
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

class ClothingGraphicalLodParameters_0p4Factory : public NxParameterized::Factory
{
	static const char* const vptr;

public:
	virtual NxParameterized::Interface* create(NxParameterized::Traits* paramTraits)
	{
		// placement new on this class using mParameterizedTraits

		void* newPtr = paramTraits->alloc(sizeof(ClothingGraphicalLodParameters_0p4), ClothingGraphicalLodParameters_0p4::ClassAlignment);
		if (!NxParameterized::IsAligned(newPtr, ClothingGraphicalLodParameters_0p4::ClassAlignment))
		{
			NX_PARAM_TRAITS_WARNING(paramTraits, "Unaligned memory allocation for class ClothingGraphicalLodParameters_0p4");
			paramTraits->free(newPtr);
			return 0;
		}

		memset(newPtr, 0, sizeof(ClothingGraphicalLodParameters_0p4)); // always initialize memory allocated to zero for default values
		return NX_PARAM_PLACEMENT_NEW(newPtr, ClothingGraphicalLodParameters_0p4)(paramTraits);
	}

	virtual NxParameterized::Interface* finish(NxParameterized::Traits* paramTraits, void* bufObj, void* bufStart, physx::PxI32* refCount)
	{
		if (!NxParameterized::IsAligned(bufObj, ClothingGraphicalLodParameters_0p4::ClassAlignment)
		        || !NxParameterized::IsAligned(bufStart, ClothingGraphicalLodParameters_0p4::ClassAlignment))
		{
			NX_PARAM_TRAITS_WARNING(paramTraits, "Unaligned memory allocation for class ClothingGraphicalLodParameters_0p4");
			return 0;
		}

		// Init NxParameters-part
		// We used to call empty constructor of ClothingGraphicalLodParameters_0p4 here
		// but it may call default constructors of members and spoil the data
		NX_PARAM_PLACEMENT_NEW(bufObj, NxParameterized::NxParameters)(paramTraits, bufStart, refCount);

		// Init vtable (everything else is already initialized)
		*(const char**)bufObj = vptr;

		return (ClothingGraphicalLodParameters_0p4*)bufObj;
	}

	virtual const char* getClassName()
	{
		return (ClothingGraphicalLodParameters_0p4::staticClassName());
	}

	virtual physx::PxU32 getVersion()
	{
		return (ClothingGraphicalLodParameters_0p4::staticVersion());
	}

	virtual physx::PxU32 getAlignment()
	{
		return (ClothingGraphicalLodParameters_0p4::ClassAlignment);
	}

	virtual const physx::PxU32* getChecksum(physx::PxU32& bits)
	{
		return (ClothingGraphicalLodParameters_0p4::staticChecksum(bits));
	}
};
#endif // NX_PARAMETERIZED_ONLY_LAYOUTS

} // namespace apex
} // namespace physx

#pragma warning(pop)

#endif

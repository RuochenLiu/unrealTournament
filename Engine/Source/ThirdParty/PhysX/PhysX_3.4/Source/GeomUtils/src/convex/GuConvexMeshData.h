// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2016 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#ifndef GU_CONVEX_MESH_DATA_H
#define GU_CONVEX_MESH_DATA_H

#include "foundation/PxPlane.h"
#include "PsIntrinsics.h"
#include "GuSerialize.h"
#include "GuCenterExtents.h"
#include "foundation/PxBitAndData.h"

// Data definition

namespace physx
{
namespace Gu
{
	struct BigConvexRawData;

	struct HullPolygonData
	{
	//= ATTENTION! =====================================================================================
	// Changing the data layout of this class breaks the binary serialization format.  See comments for 
	// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
	// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
	// accordingly.
	//==================================================================================================

		// PT: this structure won't be allocated with PX_NEW because polygons aren't allocated alone (with a dedicated alloc).
		// Instead they are part of a unique allocation/buffer containing all data for the ConvexHullData class (polygons, followed by
		// hull vertices, edge data, etc). As a result, ctors for embedded classes like PxPlane won't be called.

		PxPlane	mPlane;			//!< Plane equation for this polygon	//Could drop 4th elem as it can be computed from any vertex as: d = - p.dot(n);
		PxU16	mVRef8;			//!< Offset of vertex references in hull vertex data (CS: can we assume indices are tightly packed and offsets are ascending?? DrawObjects makes and uses this assumption)
		PxU8	mNbVerts;		//!< Number of vertices/edges in the polygon
		PxU8	mMinIndex;		//!< Index of the polygon vertex that has minimal projection along this plane's normal.

		PX_FORCE_INLINE	PxReal getMin(const PxVec3* PX_RESTRICT hullVertices) const	//minimum of projection of the hull along this plane normal
		{ 
			return mPlane.n.dot(hullVertices[mMinIndex]);
		}

		PX_FORCE_INLINE	PxReal getMax() const		{ return -mPlane.d; }	//maximum of projection of the hull along this plane normal
	};

	PX_FORCE_INLINE void flipData(Gu::HullPolygonData& data)
	{
		flip(data.mPlane.n.x);
		flip(data.mPlane.n.y);
		flip(data.mPlane.n.z);
		flip(data.mPlane.d);
		flip(data.mVRef8);
	}
	// PT: if this one breaks, please make sure the 'flipData' function is properly updated.
	PX_COMPILE_TIME_ASSERT(sizeof(Gu::HullPolygonData) == 20);

// TEST_INTERNAL_OBJECTS
	struct InternalObjectsData
	{
	//= ATTENTION! =====================================================================================
	// Changing the data layout of this class breaks the binary serialization format.  See comments for 
	// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
	// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
	// accordingly.
	//==================================================================================================
		PxReal	mRadius;
		PxReal	mExtents[3];

		PX_FORCE_INLINE	void reset()
		{
			mRadius = 0.0f;
			mExtents[0] = 0.0f;
			mExtents[1] = 0.0f;
			mExtents[2] = 0.0f;
		}
	};
	PX_COMPILE_TIME_ASSERT(sizeof(Gu::InternalObjectsData) == 16);
//~TEST_INTERNAL_OBJECTS

	struct ConvexHullData
	{
	//= ATTENTION! =====================================================================================
	// Changing the data layout of this class breaks the binary serialization format.  See comments for 
	// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
	// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
	// accordingly.
	//==================================================================================================

		// PT: WARNING: bounds must be followed by at least 32bits of data for safe SIMD loading
		CenterExtents		mAABB;				//!< bounds TODO: compute this on the fly from first 6 vertices in the vertex array.  We'll of course need to sort the most extreme ones to the front.
		PxVec3				mCenterOfMass;		//in local space of mesh!

		// PT: WARNING: mNbHullVertices *must* appear before mBigConvexRawData for ConvX to be able to do "big raw data" surgery
	
		PxBitAndWord		mNbEdges;			//!<the highest bit indicate whether we have grb data, the other 15 bits indicate the number of edges in this convex hull

		PxU8				mNbHullVertices;	//!< Number of vertices in the convex hull
		PxU8				mNbPolygons;		//!< Number of planar polygons composing the hull

		HullPolygonData*	mPolygons;			//!< Array of mNbPolygons structures
		BigConvexRawData*	mBigConvexRawData;	//!< Hill climbing data, only for large convexes! else NULL.

// TEST_INTERNAL_OBJECTS
		InternalObjectsData	mInternal;
//~TEST_INTERNAL_OBJECTS

		PX_FORCE_INLINE ConvexHullData(const PxEMPTY) : mNbEdges(PxEmpty)
		{
		}

		PX_FORCE_INLINE ConvexHullData()
		{
		}

		PX_FORCE_INLINE	const CenterExtentsPadded& getPaddedBounds() const
		{
			// PT: see compile-time assert at the end of file
			return static_cast<const CenterExtentsPadded&>(mAABB);
		}

		PX_FORCE_INLINE	const PxVec3* getHullVertices()	const	//!< Convex hull vertices
		{
			const char* tmp = reinterpret_cast<const char*>(mPolygons);
			tmp += sizeof(Gu::HullPolygonData) * mNbPolygons;
			return reinterpret_cast<const PxVec3*>(tmp);
		}

		PX_FORCE_INLINE	const PxU8* getFacesByEdges8()	const	//!< for each edge, gives 2 adjacent polygons; used by convex-convex code to come up with all the convex' edge normals.  
		{
			const char* tmp = reinterpret_cast<const char*>(mPolygons);
			tmp += sizeof(Gu::HullPolygonData) * mNbPolygons;
			tmp += sizeof(PxVec3) * mNbHullVertices; 
			return reinterpret_cast<const PxU8*>(tmp);
		}

		PX_FORCE_INLINE	const PxU8* getFacesByVertices8()	const	//!< for each edge, gives 2 adjacent polygons; used by convex-convex code to come up with all the convex' edge normals.  
		{
			const char* tmp = reinterpret_cast<const char*>(mPolygons);
			tmp += sizeof(Gu::HullPolygonData) * mNbPolygons;
			tmp += sizeof(PxVec3) * mNbHullVertices;
			tmp += sizeof(PxU8) * mNbEdges * 2;
			return reinterpret_cast<const PxU8*>(tmp);
		}

		//If we don't build the convex hull with grb data, we will return NULL pointer
		PX_FORCE_INLINE	const PxU16*	getVerticesByEdges16()	const	//!< Vertex indices indexed by unique edges
		{
			if (mNbEdges.isBitSet())
			{
				const char* tmp = reinterpret_cast<const char*>(mPolygons);
				tmp += sizeof(Gu::HullPolygonData) * mNbPolygons;
				tmp += sizeof(PxVec3) * mNbHullVertices;
				tmp += sizeof(PxU8) * mNbEdges * 2;
				tmp += sizeof(PxU8) * mNbHullVertices * 3;
				return reinterpret_cast<const PxU16*>(tmp);
			}
			return NULL;
		}

		PX_FORCE_INLINE	const PxU8*	getVertexData8()	const	//!< Vertex indices indexed by hull polygons
		{
			const char* tmp = reinterpret_cast<const char*>(mPolygons);
			tmp += sizeof(Gu::HullPolygonData) * mNbPolygons;
			tmp += sizeof(PxVec3) * mNbHullVertices;
			tmp += sizeof(PxU8) * mNbEdges * 2;
			tmp += sizeof(PxU8) * mNbHullVertices * 3;
			if (mNbEdges.isBitSet())
				tmp += sizeof(PxU16) * mNbEdges * 2;
			return reinterpret_cast<const PxU8*>(tmp);
		}

	};
	#if PX_P64_FAMILY
	PX_COMPILE_TIME_ASSERT(sizeof(Gu::ConvexHullData) == 72);
	#else
	PX_COMPILE_TIME_ASSERT(sizeof(Gu::ConvexHullData) == 64);
	#endif

	// PT: 'getPaddedBounds()' is only safe if we make sure the bounds member is followed by at least 32bits of data
	PX_COMPILE_TIME_ASSERT(PX_OFFSET_OF(Gu::ConvexHullData, mCenterOfMass)>=PX_OFFSET_OF(Gu::ConvexHullData, mAABB)+4);

} // namespace Gu

}

//#pragma PX_POP_PACK

#endif

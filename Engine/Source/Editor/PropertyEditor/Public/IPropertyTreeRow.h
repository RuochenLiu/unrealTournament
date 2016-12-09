// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

class FPropertyPath;

class IPropertyTreeRow
{
public: 

	virtual TSharedPtr< FPropertyPath > GetPropertyPath() const = 0;

	virtual bool IsCursorHovering() const = 0;
};

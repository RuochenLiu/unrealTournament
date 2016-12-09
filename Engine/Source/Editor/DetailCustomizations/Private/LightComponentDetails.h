// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class IDetailLayoutBuilder;
class IPropertyHandle;

class FLightComponentDetails : public IDetailCustomization
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** IDetailCustomization interface */
	virtual void CustomizeDetails( IDetailLayoutBuilder& DetailBuilder ) override;

private:

	/** Helper functions which tell whether the various custom controls are enabled or not */
	bool IsLightBrightnessEnabled() const;
	bool IsUseIESBrightnessEnabled() const;
	bool IsIESBrightnessScaleEnabled() const;


private:
	TSharedPtr<IPropertyHandle> IESBrightnessTextureProperty;
	TSharedPtr<IPropertyHandle> IESBrightnessEnabledProperty;
	TSharedPtr<IPropertyHandle> IESBrightnessScaleProperty;
	TSharedPtr<IPropertyHandle> LightIntensityProperty;
};

// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "DetailCustomizationsPrivatePCH.h"
#include "SkyLightComponentDetails.h"
#include "Components/LightComponentBase.h"
#include "Engine/SkyLight.h"
#include "Components/SkyLightComponent.h"

#define LOCTEXT_NAMESPACE "SkyLightComponentDetails"



TSharedRef<IDetailCustomization> FSkyLightComponentDetails::MakeInstance()
{
	return MakeShareable( new FSkyLightComponentDetails );
}

void FSkyLightComponentDetails::CustomizeDetails( IDetailLayoutBuilder& DetailLayout )
{
	// Mobility property is on the scene component base class not the light component and that is why we have to use USceneComponent::StaticClass
	TSharedRef<IPropertyHandle> MobilityHandle = DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(USkyLightComponent, Mobility), USceneComponent::StaticClass());
	// Set a mobility tooltip specific to lights
	MobilityHandle->SetToolTipText(LOCTEXT("SkyLightMobilityTooltip", "Mobility for sky light components determines what rendering methods will be used.  A Stationary sky light has its shadowing baked into Bent Normal AO by Lightmass, but its lighting can be changed in game."));

	TSharedPtr<IPropertyHandle> LightIntensityProperty = DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(USkyLightComponent, Intensity), ULightComponentBase::StaticClass());

	if( LightIntensityProperty->IsValidHandle() )
	{
		// Point lights need to override the ui min and max for units of lumens, so we have to undo that
		LightIntensityProperty->GetProperty()->SetMetaData("UIMin", TEXT("0.0f"));
		LightIntensityProperty->GetProperty()->SetMetaData("UIMax", TEXT("20.0f"));
	}

	const TArray< TWeakObjectPtr<UObject> >& SelectedObjects = DetailLayout.GetDetailsView().GetSelectedObjects();

	for( int32 ObjectIndex = 0; ObjectIndex < SelectedObjects.Num(); ++ObjectIndex )
	{
		const TWeakObjectPtr<UObject>& CurrentObject = SelectedObjects[ObjectIndex];
		if ( CurrentObject.IsValid() )
		{
			ASkyLight* CurrentCaptureActor = Cast<ASkyLight>(CurrentObject.Get());
			if (CurrentCaptureActor != NULL)
			{
				SkyLight = CurrentCaptureActor;
				break;
			}
		}
	}

	DetailLayout.EditCategory( "SkyLight" )
	.AddCustomRow( NSLOCTEXT("SkyLightDetails", "UpdateSkyLight", "Recapture Scene") )
	[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.FillWidth(1.f)
		.Padding(10,5)
		[
			SNew(SButton)
			.ContentPadding(3)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.OnClicked( this, &FSkyLightComponentDetails::OnUpdateSkyCapture )
			.Text( NSLOCTEXT("SkyLightDetails", "UpdateSkyCapture", "Recapture Scene") )
		]
	];
}

FReply FSkyLightComponentDetails::OnUpdateSkyCapture()
{
	if( SkyLight.IsValid() )
	{
		if (UWorld* SkyLightWorld = SkyLight->GetWorld())
		{
			SkyLightWorld->UpdateAllSkyCaptures();
		}
	}

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE

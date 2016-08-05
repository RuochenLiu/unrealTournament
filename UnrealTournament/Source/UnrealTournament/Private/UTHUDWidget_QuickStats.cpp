// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#include "UnrealTournament.h"
#include "UTHUDWidget_QuickStats.h"
#include "UTArmor.h"
#include "UTJumpBoots.h"
#include "UTPlaceablePowerup.h"
#include "UTWeapon.h"
#include "UTTimedPowerup.h"
#include "UTCTFRoundGameState.h"
#include "UTCTFMajorMessage.h"

const float BOUNCE_SCALE = 1.6f;			
const float BOUNCE_TIME = 1.2f;		// SHould be (BOUNCE_SCALE - 1.0) * # of seconds

UUTHUDWidget_QuickStats::UUTHUDWidget_QuickStats(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	Position=FVector2D(0.0f, 0.0f);
	Size=FVector2D(0.0f,0.0f);
	ScreenPosition=FVector2D(0.5f, 0.5f);
	Origin=FVector2D(0.f,0.0f);
	DesignedResolution=1080;

	RallyAnimTimers.Add(RALLY_ANIMATION_TIME * 0.25);
	RallyAnimTimers.Add(RALLY_ANIMATION_TIME * 0.5);
	RallyAnimTimers.Add(RALLY_ANIMATION_TIME * 0.75);

	FlagInfo.bIsInfo = true;
	BootsInfo.bIsInfo = true;
	PowerupInfo.bIsInfo = true;
	BoostProvidedPowerupInfo.bIsInfo = true;
}

void UUTHUDWidget_QuickStats::InitializeWidget(AUTHUD* Hud)
{
	Super::InitializeWidget(Hud);

	DrawAngle = 0.0f;
	CurrentLayoutIndex = 0;
	HealthInfo.TextColor = FLinearColor(0.4f, 0.95f, 0.48f, 1.0f);
	HealthInfo.IconColor = HealthInfo.TextColor;
	FlagInfo.bNoText = false;
}

bool UUTHUDWidget_QuickStats::ShouldDraw_Implementation(bool bShowScores)
{
	return Super::ShouldDraw_Implementation(bShowScores) && !UTHUDOwner->bShowComsMenu && !UTHUDOwner->bShowWeaponWheel && (!UTHUDOwner->GetQuickStatsHidden() || !UTHUDOwner->GetQuickInfoHidden());
}

FVector2D UUTHUDWidget_QuickStats::CalcRotOffset(FVector2D InitialPosition, float Angle)
{
	float Sin = 0.f;
	float Cos = 0.f;

	FMath::SinCos(&Sin, &Cos, FMath::DegreesToRadians(Angle - 180.0f));
	FVector2D NewPoint;
	
	NewPoint.X = InitialPosition.X * Cos - InitialPosition.Y * Sin;
	NewPoint.Y = InitialPosition.Y * Cos + InitialPosition.X * Sin;
	return NewPoint;
}

void UUTHUDWidget_QuickStats::PreDraw(float DeltaTime, AUTHUD* InUTHUDOwner, UCanvas* InCanvas, FVector2D InCanvasCenter)
{
	// Look to see if we should draw the ammo...
	DrawAngle = InUTHUDOwner->GetQuickStatsAngle();

	float DrawDistance = InUTHUDOwner->GetQuickStatsDistance();
	FName DisplayTag = InUTHUDOwner->GetQuickStatsType();
	DrawScale = InUTHUDOwner->GetQuickStatScaleOverride();

	HorizontalBackground.RenderOpacity = InUTHUDOwner->GetQuickStatsBackgroundAlpha();
	VerticalBackground.RenderOpacity = InUTHUDOwner->GetQuickStatsBackgroundAlpha();

	ForegroundOpacity = InUTHUDOwner->GetQuickStatsForegroundAlpha();

	if (DisplayTag != NAME_None && DisplayTag != CurrentLayoutTag)
	{
		for (int32 i=0; i < Layouts.Num(); i++)
		{
			if (Layouts[i].Tag == DisplayTag)
			{
				CurrentLayoutIndex = i;
				CurrentLayoutTag = DisplayTag;
				break;
			}
		}
	}

	Position = CalcRotatedDrawLocation( 1920.0f * DrawDistance, DrawAngle);
	Super::PreDraw(DeltaTime, InUTHUDOwner, InCanvas, InCanvasCenter);

	AUTCharacter* CharOwner = Cast<AUTCharacter>(UTHUDOwner->UTPlayerOwner->GetViewTarget());
	AUTPlayerState* UTPlayerState = CharOwner != nullptr ? Cast<AUTPlayerState>(CharOwner->PlayerState) : nullptr;
	FlagInfo.Value = 0;
	if (CharOwner && UTPlayerState)
	{
		AUTWeapon* Weap = CharOwner->GetWeapon();

		WeaponColor = Weap ? Weap->IconColor : FLinearColor::White;
		bHorizBorders = Layouts[CurrentLayoutIndex].bHorizontalBorder;

		// ----------------- Ammo
		AmmoInfo.Value = Weap ? Weap->Ammo : 0;
		AmmoInfo.bInfinite = Weap && !Weap->NeedsAmmoDisplay();
		AmmoInfo.IconColor = WeaponColor;

		if (CheckStatForUpdate(DeltaTime, AmmoInfo, Weap == LastWeapon) )
		{
			// Ammo changed, Add some scale...
			AmmoInfo.Animate(StatAnimTypes::Scale, 1.5f, 3.25f, 1.0f, true);
		}

		LastWeapon = Weap;
		if (Weap && !AmmoInfo.bInfinite && Weap->AmmoWarningAmount > 0)
		{
			float AmmoPerc = float(AmmoInfo.Value-1) / float(Weap->MaxAmmo);
			GetHighlightStrength(AmmoInfo, AmmoPerc, float(Weap->AmmoWarningAmount) / float(Weap->MaxAmmo));
		}
		else
		{
			AmmoInfo.HighlightStrength = 0.0f;
		}

		// ----------------- Health
		HealthInfo.Value = CharOwner->Health;
		HealthInfo.bInfinite = false;

		if (HealthInfo.Value < HealthInfo.LastValue)
		{
			HealthInfo.Flash(FLinearColor(1.0f, 0.0f, 0.0f, 1.0f), 0.8f);
			HealthInfo.Animate(StatAnimTypes::Scale, 1.5f, 3.25f, 1.0f, true);
			HealthInfo.LastValue = HealthInfo.Value;
		
		}
		else if ( CheckStatForUpdate(DeltaTime, HealthInfo) )
		{
			HealthInfo.Animate(StatAnimTypes::Scale, 1.5f, 3.25f, 1.0f, true);
		}

		float HealthPerc = float(HealthInfo.Value) / 100.0f;
		GetHighlightStrength(HealthInfo, HealthPerc, 0.65f);

		// ----------------- Armor / Inventory
		bool bArmorVisible = (CharOwner->GetArmorAmount() > 0);
		bool bBootsVisible = false;
		BootsInfo.Value = 0;
		ArmorInfo.Value = CharOwner->GetArmorAmount();
		AUTTimedPowerup* CurrentPowerup = nullptr;

		for (TInventoryIterator<> It(CharOwner); It; ++It)
		{
			AUTJumpBoots* Boots = Cast<AUTJumpBoots>(*It);
			if (Boots != nullptr)
			{
				BootsInfo.Value = Boots->NumJumps;
				int32 MaxJumps = Boots->StaticClass()->GetDefaultObject<AUTJumpBoots>()->NumJumps;
				BootsInfo.HighlightStrength = MaxJumps > 0 ? float(Boots->NumJumps) / float(MaxJumps) : 0.0f;
				bBootsVisible = true;
			}

			if (CurrentPowerup == nullptr)
			{
				CurrentPowerup = Cast<AUTTimedPowerup>(*It);

				//Ignore powerups given by a boost class as those are handled separately
				if (CurrentPowerup && CurrentPowerup->bBoostPowerupSuppliedItem)
				{
					CurrentPowerup = nullptr;
				}
			}
		}
	
		ArmorInfo.OverlayTextures.Empty();
		ArmorInfo.bUseOverlayTexture = true;
		if (CharOwner && CharOwner->bIsWearingHelmet)
		{
			ArmorInfo.OverlayTextures.Add(HelmetIcon);
		}

		if (bArmorVisible != ArmorInfo.bVisible)
		{
			if (bArmorVisible)
			{
				ArmorInfo.Animate(StatAnimTypes::Scale, 1.5f, 0.0f, 1.0f, true);
				ArmorInfo.LastValue = ArmorInfo.Value;
			}

			ArmorInfo.bVisible = bArmorVisible;
		}

		ArmorInfo.bInfinite = false;

		if (ArmorInfo.Value < ArmorInfo.LastValue)
		{
			ArmorInfo.Flash(FLinearColor(1.0f, 0.0f, 0.0f, 1.0f), 0.8f);
			ArmorInfo.Animate(StatAnimTypes::Scale, 1.5f, 3.25f, 1.0f, true);
			ArmorInfo.LastValue = ArmorInfo.Value;
		
		}
		else if ( CheckStatForUpdate(DeltaTime, ArmorInfo)  )
		{
			ArmorInfo.Animate(StatAnimTypes::Scale, 1.5f, 3.25f, 1.0f, true);
		}

		float ArmorPerc = ArmorInfo.Value > 0 ? float(ArmorInfo.Value) / 100.0f : 1.0f;
		ArmorInfo.HighlightStrength = 0.0f;
		ArmorInfo.IconColor = FLinearColor::Yellow;

		if (bBootsVisible!= BootsInfo.bVisible)
		{
			if (bBootsVisible)
			{
				BootsInfo.Animate(StatAnimTypes::Scale, 1.5f, 0.0f, 1.0f, true);
				BootsInfo.LastValue = ArmorInfo.Value;
				BootsInfo.HighlightStrength = 1.0f;
			}

			BootsInfo.bVisible = bBootsVisible;
		}

		if ( CheckStatForUpdate(DeltaTime, BootsInfo) )
		{
			BootsInfo.Animate(StatAnimTypes::Scale, 1.5f, 3.25f, 1.0f, true);
		}

		PowerupInfo.bAltIcon = false;
		PowerupInfo.bUseLabel = false;
		PowerupInfo.Label = FText::GetEmpty();
		PowerupInfo.Value = 0;
		if (CurrentPowerup != nullptr)
		{
			PowerupInfo.Value = int32(CurrentPowerup->TimeRemaining);

			PowerupIcon.Atlas = CurrentPowerup->HUDIcon.Texture;
			PowerupIcon.UVs.U = CurrentPowerup->HUDIcon.U;
			PowerupIcon.UVs.V = CurrentPowerup->HUDIcon.V;
			PowerupIcon.UVs.UL = CurrentPowerup->HUDIcon.UL;
			PowerupIcon.UVs.VL = CurrentPowerup->HUDIcon.VL;

			PowerupIcon.Position.Y = -22;
			PowerupIcon.RenderOffset = FVector2D(0.5f, 0.5f);
			PowerupIcon.Size = FVector2D(PowerupIcon.UVs.UL * 1.07f, 35);

			if (PowerupInfo.Value <= 5)
			{
				PowerupInfo.HighlightStrength = float(PowerupInfo.Value) / 5.0f;
			}
		}

		if (UTPlayerState->BoostClass)
		{
			BoostProvidedPowerupInfo.bUseLabel = true;
			BoostProvidedPowerupInfo.Label = FText::GetEmpty();
			BoostProvidedPowerupInfo.Value = 0;
			BoostProvidedPowerupInfo.HighlightStrength = 0.f;

			AUTInventory* InventoryItem = UTPlayerState->BoostClass->GetDefaultObject<AUTInventory>();
			BoostIcon.UVs.U = InventoryItem->HUDIcon.U;
			BoostIcon.UVs.V = InventoryItem->HUDIcon.V;
			BoostIcon.UVs.UL = InventoryItem->HUDIcon.UL;
			BoostIcon.UVs.VL = InventoryItem->HUDIcon.VL;
			BoostIcon.Atlas = InventoryItem->HUDIcon.Texture;
			BoostIcon.Size = FVector2D(InventoryItem->HUDIcon.UL * 1.07f, 35);
			
			AUTInventory* ActiveBoost = nullptr;
			for (TInventoryIterator<> It(CharOwner); It; ++It)
			{
				AUTInventory* InventoryItemIt = Cast<AUTInventory>(*It);
				if (InventoryItemIt && InventoryItemIt->bBoostPowerupSuppliedItem)
				{
					ActiveBoost = InventoryItemIt;
				}
			}

			if (ActiveBoost)
			{
				BoostProvidedPowerupInfo.HighlightStrength = 1.f;
				BoostProvidedPowerupInfo.bUseLabel = false;

				AUTTimedPowerup* TimedPowerup = Cast<AUTTimedPowerup>(ActiveBoost);
				if (TimedPowerup)
				{
					BoostProvidedPowerupInfo.Value = static_cast<int>(TimedPowerup->TimeRemaining);
				}

				AUTWeapon* WeaponPowerup = Cast<AUTWeapon>(ActiveBoost);
				if (WeaponPowerup)
				{
					BoostProvidedPowerupInfo.Value = WeaponPowerup->Ammo;
				}

				AUTPlaceablePowerup* PlaceablePowerup = Cast<AUTPlaceablePowerup>(ActiveBoost);
				if (PlaceablePowerup)
				{
					BoostProvidedPowerupInfo.Value = UTPlayerState->GetRemainingBoosts();
				}
			}
			else
			{
				//Show info for actively having access to a powerup
				if (UTPlayerState->GetRemainingBoosts() >= 1)
				{
					BoostProvidedPowerupInfo.Value = UTPlayerState->GetRemainingBoosts();
					BoostProvidedPowerupInfo.HighlightStrength = 0.5f;
					if (UTHUDOwner->UTPlayerOwner->bNeedsBoostNotify)
					{
						UTHUDOwner->UTPlayerOwner->bNeedsBoostNotify = false;
						BoostProvidedPowerupInfo.Animate(StatAnimTypes::Scale, 2.0f, 10.f, 1.0f, true);
					}
					else if (!BoostProvidedPowerupInfo.IsAnimationTypeAlreadyPlaying(StatAnimTypes::Scale))
					{
						BoostProvidedPowerupInfo.Animate(StatAnimTypes::Scale, 2.f, 3.25f, 1.0f, true);
					}
					BoostProvidedPowerupInfo.Label = UTHUDOwner->BoostLabel;
				}
				else
				{
					BoostProvidedPowerupInfo.Value = 0.0f;
/*
					//Show countdown to power up
					AUTCTFRoundGameState* RoundGameState = GetWorld()->GetGameState<AUTCTFRoundGameState>();
					if (RoundGameState != NULL && RoundGameState->IsTeamAbleToEarnPowerup(UTPlayerState->GetTeamNum()))
					{
						BoostProvidedPowerupInfo.Label = FText::FromString(FString::Printf(TEXT("Kill: %i"), RoundGameState->GetKillsNeededForPowerup(UTPlayerState->GetTeamNum())));
						BoostProvidedPowerupInfo.Value = RoundGameState->GetKillsNeededForPowerup(UTPlayerState->GetTeamNum());
					}
					else if (UTGameState != NULL && UTGameState->BoostRechargeMaxCharges > 0 && UTGameState->BoostRechargeTime > 0.0f)
					{
						BoostProvidedPowerupInfo.Label = FText::FromString(FString::Printf(TEXT("CD: %i"), FMath::CeilToInt(UTPlayerState->BoostRechargeTimeRemaining)));
						BoostProvidedPowerupInfo.Value = FMath::CeilToInt(UTPlayerState->BoostRechargeTimeRemaining);
					}
*/
				}
			}
		}

		bool bPlayerCanRally = UTHUDOwner->UTPlayerOwner->CanPerformRally();
		AUTCTFGameState* GameState = GetWorld()->GetGameState<AUTCTFGameState>();
		bool bShowTimer = !bPlayerCanRally && !UTPlayerState->CarriedObject && UTPlayerState->Team && GameState && GameState->bAttackersCanRally && ((UTPlayerState->Team->TeamIndex == 0) == GameState->bRedToCap) && (UTPlayerState->CarriedObject == nullptr) && CharOwner && CharOwner->bCanRally && (UTPlayerState->RemainingRallyDelay > 0);

		if (UTPlayerState->CarriedObject != nullptr || bPlayerCanRally || bShowTimer)
		{
			FlagInfo.bCustomIconUnderlay = false;
			FlagInfo.OverlayTextures.Empty();
			FlagInfo.bUseLabel = true;

			FlagInfo.Value = 1;
			bool bWantsPulse = true; 

			if (bPlayerCanRally || bShowTimer)
			{
				FlagInfo.IconColor = FLinearColor::Yellow;
				FlagInfo.Label = UTHUDOwner->RallyLabel;
			}
			else
			{
				FlagInfo.IconColor = UTPlayerState->CarriedObject->GetTeamNum() == 0 ? FLinearColor::Red : FLinearColor::Blue;
				FlagInfo.Label = FText::GetEmpty();
			}

			FlagInfo.HighlightStrength = 1.f;
			FlagInfo.bUseOverlayTexture = false;

			if (bPlayerCanRally)
			{
				FlagInfo.bCustomIconUnderlay = true;
				if (UTPlayerState->CarriedObject)
				{
					if (InUTHUDOwner && InUTHUDOwner->UTPlayerOwner && (GetWorld()->GetTimeSeconds() - InUTHUDOwner->UTPlayerOwner->LastRallyRequestTime < 10.5f))
					{
						FlagInfo.Label = FText::GetEmpty();
						bWantsPulse = false;
					}
				}

				if (UTHUDOwner->UTPlayerOwner->bNeedsRallyNotify)
				{
					if (bWantsPulse)
					{
						FlagInfo.Animate(StatAnimTypes::Scale, 2.0f, 10.f, 1.0f, true);
						UTHUDOwner->UTPlayerOwner->ClientReceiveLocalizedMessage(UUTCTFMajorMessage::StaticClass(), 23);
					}
					UTHUDOwner->UTPlayerOwner->bNeedsRallyNotify = false;
				}
				else if (bWantsPulse && !FlagInfo.IsAnimationTypeAlreadyPlaying(StatAnimTypes::Scale))
				{
					FlagInfo.Animate(StatAnimTypes::Scale, 2.0f, 3.25f, 1.0f, true);
				}
			}
			else if (bShowTimer)
			{
				FlagInfo.Label = FText::AsNumber(int32(UTPlayerState->RemainingRallyDelay));
			}

			if (UTPlayerState->CarriedObject != nullptr)
			{
				AUTCTFFlag* CTFFlag = Cast<AUTCTFFlag>(UTPlayerState->CarriedObject);
				if (CTFFlag != nullptr && CTFFlag->bCurrentlyPinged)
				{
					FlagInfo.bUseOverlayTexture = true;
					FlagInfo.OverlayTextures.Add(DetectedIcon);
				}
			}
		}
		else
		{
			FlagInfo.Value = 0;
		}
	}

	HealthInfo.UpdateAnimation(DeltaTime);
	AmmoInfo.UpdateAnimation(DeltaTime);
	ArmorInfo.UpdateAnimation(DeltaTime);
	FlagInfo.UpdateAnimation(DeltaTime);
	BootsInfo.UpdateAnimation(DeltaTime);
	PowerupInfo.UpdateAnimation(DeltaTime);
	BoostProvidedPowerupInfo.UpdateAnimation(DeltaTime);
}

bool UUTHUDWidget_QuickStats::CheckStatForUpdate(float DeltaTime, FStatInfo& Stat, bool bLookForChange)
{
	if (Stat.Value > Stat.LastValue && bLookForChange)
	{
		Stat.LastValue = Stat.Value;
		return true;
	}

	Stat.LastValue = Stat.Value;
	return false;
}

FVector2D UUTHUDWidget_QuickStats::GetBoostLocation()
{
	return Layouts[CurrentLayoutIndex].bFollowRotation ? CalcRotOffset(Layouts[CurrentLayoutIndex].BoostProvidedPowerupOffset, DrawAngle) : Layouts[CurrentLayoutIndex].BoostProvidedPowerupOffset;
}


void UUTHUDWidget_QuickStats::Draw_Implementation(float DeltaTime)
{

	RenderDelta = DeltaTime;

	bool bFollowRotation = Layouts[CurrentLayoutIndex].bFollowRotation;
	DrawStat(bFollowRotation ? CalcRotOffset(Layouts[CurrentLayoutIndex].HealthOffset,DrawAngle) : Layouts[CurrentLayoutIndex].HealthOffset, HealthInfo, HealthIcon);
	DrawStat(bFollowRotation ? CalcRotOffset(Layouts[CurrentLayoutIndex].ArmorOffset,DrawAngle) : Layouts[CurrentLayoutIndex].ArmorOffset, ArmorInfo, ArmorInfo.bAltIcon ? ArmorIconWithShieldBelt : ArmorIcon);
	DrawStat(bFollowRotation ? CalcRotOffset(Layouts[CurrentLayoutIndex].AmmoOffset,DrawAngle) : Layouts[CurrentLayoutIndex].AmmoOffset, AmmoInfo, AmmoIcon);

	if (FlagInfo.Value > 0)
	{
		DrawStat(bFollowRotation ? CalcRotOffset(Layouts[CurrentLayoutIndex].FlagOffset, DrawAngle) : Layouts[CurrentLayoutIndex].FlagOffset, FlagInfo, FlagIcon);
	}

	if (BootsInfo.Value > 0)
	{
		DrawStat(bFollowRotation ? CalcRotOffset(Layouts[CurrentLayoutIndex].BootsOffset, DrawAngle) : Layouts[CurrentLayoutIndex].BootsOffset,	BootsInfo, BootsIcon);
	}

	if (PowerupInfo.Value > 0)
	{
		DrawStat(bFollowRotation ? CalcRotOffset(Layouts[CurrentLayoutIndex].PowerupOffset, DrawAngle) : Layouts[CurrentLayoutIndex].AmmoOffset, PowerupInfo, PowerupIcon);
	}

	if (BoostProvidedPowerupInfo.Value > 0)
	{
		DrawStat(GetBoostLocation(), BoostProvidedPowerupInfo, BoostIcon );
	}
}

void UUTHUDWidget_QuickStats::DrawStat(FVector2D StatOffset, FStatInfo& Info, FHUDRenderObject_Texture Icon)
{
	if (!Info.bVisible) return;
	if (Info.bIsInfo && UTHUDOwner->GetQuickInfoHidden()) return;
	if (!Info.bIsInfo && UTHUDOwner->GetQuickStatsHidden()) return;

	if (bHorizBorders)
	{
		HorizontalBackground.RenderColor = Info.BackgroundColor;
		HorizontalBackground.RenderOpacity *= Info.Opacity;
		RenderObj_Texture(HorizontalBackground, StatOffset);

		if (Info.HighlightStrength> 0)
		{

			float HighlightStrength = FMath::Clamp<float>(Info.HighlightStrength, 0.0f, 1.0f);

			HorizontalHighlight.RenderColor = FLinearColor(0.82f,0.0f,0.0f,1.0f);
			HorizontalHighlight.RenderOpacity *= Info.Opacity;
			FVector2D DrawOffset = StatOffset;

			float Height = HorizontalBackground.Size.Y * HighlightStrength;
			HorizontalHighlight.Size.Y = Height;
			HorizontalHighlight.UVs.VL = Height;

			DrawOffset.Y -= (HorizontalBackground.Size.Y * 0.5f);
			RenderObj_Texture(HorizontalHighlight, DrawOffset);
		}

	}
	else
	{
		VerticalBackground.RenderColor = Info.BackgroundColor;
		VerticalBackground.RenderOpacity *= Info.Opacity;
		RenderObj_Texture(VerticalBackground, StatOffset);
	}

	if (Info.bCustomIconUnderlay)
	{
		DrawIconUnderlay(StatOffset);
	}

	Icon.RenderScale = Info.Scale;
	Icon.RenderColor = Info.IconColor;
	Icon.RenderOpacity = ForegroundOpacity * Info.Opacity;
	RenderObj_Texture(Icon, StatOffset);

	if (!Info.bNoText)
	{
		if (Info.bInfinite)
		{
			InfiniteIcon.RenderOpacity *= Info.Opacity;
			RenderObj_Texture(InfiniteIcon, StatOffset);
		}
		else
		{
			TextTemplate.TextScale = Info.Scale;
			TextTemplate.Text = Info.bUseLabel ? Info.Label : FText::AsNumber(Info.Value);

			FLinearColor DrawColor = Info.TextColor;
			if (Info.TextColorOverrideDuration > 0.0f)
			{
				Info.TextColorOverrideTimer += RenderDelta;
				float Perc = FMath::Clamp<float>(Info.TextColorOverrideTimer / Info.TextColorOverrideDuration, 0.0f, 1.0f);
				DrawColor.R = FMath::Lerp<float>(Info.TextColorOverride.R, Info.TextColor.R, Perc);
				DrawColor.G = FMath::Lerp<float>(Info.TextColorOverride.G, Info.TextColor.G, Perc);
				DrawColor.B = FMath::Lerp<float>(Info.TextColorOverride.B, Info.TextColor.B, Perc);
		
				if (Perc >= 1.0f) Info.TextColorOverrideDuration = 0.0f;
			}

			TextTemplate.RenderColor = DrawColor;
			TextTemplate.RenderOpacity = ForegroundOpacity * Info.Opacity;
			RenderObj_Text(TextTemplate, StatOffset);
		}
	}

	if (Info.bUseOverlayTexture)
	{
		for (int32 i=0; i < Info.OverlayTextures.Num(); i++)
		{
			Info.OverlayTextures[i].RenderScale = Info.OverlayScale;
			RenderObj_Texture(Info.OverlayTextures[i], StatOffset);
		}
	}
}

void UUTHUDWidget_QuickStats::GetHighlightStrength(FStatInfo& Stat, float Perc, float WarnPerc)
{
	Stat.HighlightStrength = (Perc <= WarnPerc) ? FMath::Max(0.1875f, 1.0f - (Perc / WarnPerc)) : 0.0f;
}

FLinearColor UUTHUDWidget_QuickStats::InterpColor(FLinearColor DestinationColor, float Delta)
{
	float AlphaSin = 1.0f - FMath::Abs<float>(FMath::Sin(Delta * 2.0f));
	
	float R = FMath::Lerp(DestinationColor.R, 0.0f, AlphaSin);
	float G = FMath::Lerp(DestinationColor.G, 0.0f, AlphaSin);
	float B = FMath::Lerp(DestinationColor.B, 0.0f, AlphaSin);
	return FLinearColor(R, G, B, 1.0f);
}

float UUTHUDWidget_QuickStats::GetDrawScaleOverride()
{
	return Super::GetDrawScaleOverride() * DrawScale;
}

void UUTHUDWidget_QuickStats::PingBoostWidget()
{
	BoostProvidedPowerupInfo.Animate(StatAnimTypes::Scale, 1.5f, 3.25f, 1.0f, true);		
}


void UUTHUDWidget_QuickStats::DrawIconUnderlay(FVector2D StatOffset)
{
	RallyFlagIcon.bUseTeamColors = true;
	RallyFlagIcon.RenderScale = 1.0f;

	for (int32 i = 0; i < RallyAnimTimers.Num(); i++)
	{
		RallyAnimTimers[i] += RenderDelta;
		if (RallyAnimTimers[i] > RALLY_ANIMATION_TIME) RallyAnimTimers[i] = 0;
		float Position = (RallyAnimTimers[i] / RALLY_ANIMATION_TIME);
	
		float XPos = FMath::InterpEaseOut<float>(64.0f, 0.0f, Position, 2.0f);
		RallyFlagIcon.RenderOpacity = FMath::InterpEaseOut<float>(1.0f, 0.0f, Position, 2.0f);
		RenderObj_Texture(RallyFlagIcon,StatOffset + FVector2D(XPos, 0.0f));
		RenderObj_Texture(RallyFlagIcon,StatOffset + FVector2D(XPos * -1, 0.0f));
	}
}
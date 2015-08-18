// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "UTOnlineGameSettingsBase.h"
#include "SlateBasics.h"

#if !UE_SERVER

class FServerData;

class UNREALTOURNAMENT_API SUTJoinInstance : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SUTJoinInstance)
	{}

	SLATE_ARGUMENT(TWeakObjectPtr<class UUTLocalPlayer>, PlayerOwner)			
	SLATE_ARGUMENT(TSharedPtr<FServerData>, ServerData)
	SLATE_ARGUMENT(FString, InstanceId)
	SLATE_ARGUMENT(bool, bSpectator)
	SLATE_END_ARGS()


	/** needed for every widget */
	void Construct(const FArguments& InArgs);

	// Called to cancel the quickmatch process
	void Cancel();

	virtual void TellSlateIWantKeyboardFocus();
	virtual void OnDialogClosed();

protected:

	bool bWaitingForMatch;

	/** Holds a reference to the SCanvas widget that makes up the dialog */
	TSharedPtr<SCanvas> Canvas;

	/** Holds a reference to the SOverlay that defines the content for this dialog */
	TSharedPtr<SOverlay> WindowContent;

	TSharedPtr<FServerData> ServerData;

	FString InstanceId;

	bool bSpectator;

private:

	bool bCancelJoin;

	TWeakObjectPtr<AUTServerBeaconClient> Beacon;

	IOnlineSubsystem* OnlineSubsystem;
	IOnlineIdentityPtr OnlineIdentityInterface;
	IOnlineSessionPtr OnlineSessionInterface;

	FDelegateHandle OnFindSessionCompleteHandle;
	FDelegateHandle OnCancelFindSessionCompleteHandle;

	TSharedPtr<class FUTOnlineGameSearchBase> SearchSettings;
	TWeakObjectPtr<class UUTLocalPlayer> PlayerOwner;
	FString QuickMatchType;
	float StartTime;

	FText GetStatusText() const;

	// Begin the search for a HUB to join
	void BeginJoin();
	void OnInitialFindCancel(bool bWasSuccessful);
	void OnSearchCancelled(bool bWasSuccessful);

	virtual void OnServerBeaconResult(AUTServerBeaconClient* Sender, FServerBeaconInfo ServerInfo);
	virtual void OnServerBeaconFailure(AUTServerBeaconClient* Sender);
	
	virtual void RequestJoinFromHub();

	void OnRequestJoinResult(EInstanceJoinResult::Type Result, const FString& Params);

	virtual bool SupportsKeyboardFocus() const override;

	FReply OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent);
	FReply OnCancelClick();

};

#endif
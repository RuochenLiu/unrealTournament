// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	GameInstance.cpp: Implementation of GameInstance class
=============================================================================*/


#include "EnginePrivate.h"
#include "Engine/GameInstance.h"
#include "Engine/Engine.h"
#include "Engine/DemoNetDriver.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionInterface.h"
#include "GameFramework/OnlineSession.h"

#if WITH_EDITOR
#include "UnrealEd.h"
#endif
#include "Engine/GameEngine.h"


UGameInstance::UGameInstance(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
, TimerManager(new FTimerManager())
{
}

void UGameInstance::FinishDestroy()
{
	if (TimerManager)
	{
		delete TimerManager;
		TimerManager = nullptr;
	}

	Super::FinishDestroy();
}

UWorld* UGameInstance::GetWorld() const
{
	return WorldContext ? WorldContext->World() : NULL;
}

UEngine* UGameInstance::GetEngine() const
{
	return CastChecked<UEngine>(GetOuter());
}

void UGameInstance::Init()
{
	ReceiveInit();

	const auto OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub != nullptr)
	{
		IOnlineSessionPtr SessionInt = OnlineSub->GetSessionInterface();
		if (SessionInt.IsValid())
		{
			SessionInt->AddOnSessionUserInviteAcceptedDelegate_Handle(FOnSessionUserInviteAcceptedDelegate::CreateUObject(this, &UGameInstance::HandleSessionUserInviteAccepted));
		}
	}
}

void UGameInstance::Shutdown()
{
	ReceiveShutdown();

	const auto OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub != nullptr)
	{
		IOnlineSessionPtr SessionInt = OnlineSub->GetSessionInterface();
		if (SessionInt.IsValid())
		{
			SessionInt->ClearOnSessionUserInviteAcceptedDelegate_Handle(OnSessionUserInviteAcceptedDelegateHandle);
		}
	}

	// Clear the world context pointer to prevent further access.
	WorldContext = nullptr;
}

void UGameInstance::InitializeStandalone()
{
	UGameEngine* const Engine = CastChecked<UGameEngine>(GetEngine());

	// Creates the world context. This should be the only WorldContext that ever gets created for this GameInstance.
	WorldContext = &Engine->CreateNewWorldContext(EWorldType::Game);
	WorldContext->OwningGameInstance = this;

	// In standalone create a dummy world from the beginning to avoid issues of not having a world until LoadMap gets us our real world
	UWorld* DummyWorld = UWorld::CreateWorld(EWorldType::Game, false);
	DummyWorld->SetGameInstance(this);
	WorldContext->SetCurrentWorld(DummyWorld);

	Init();
}

#if WITH_EDITOR
bool UGameInstance::InitializePIE(bool bAnyBlueprintErrors, int32 PIEInstance)
{
	UEditorEngine* const EditorEngine = CastChecked<UEditorEngine>(GetEngine());

	// Look for an existing pie world context, may have been created before
	WorldContext = EditorEngine->GetWorldContextFromPIEInstance(PIEInstance);

	if (!WorldContext)
	{
		// If not, create a new one
		WorldContext = &EditorEngine->CreateNewWorldContext(EWorldType::PIE);
		WorldContext->PIEInstance = PIEInstance;
	}

	WorldContext->OwningGameInstance = this;
	
	const FString WorldPackageName = EditorEngine->EditorWorld->GetOutermost()->GetName();

	// Establish World Context for PIE World
	WorldContext->LastURL.Map = WorldPackageName;
	WorldContext->PIEPrefix = WorldContext->PIEInstance != INDEX_NONE ? UWorld::BuildPIEPackagePrefix(WorldContext->PIEInstance) : FString();

	const ULevelEditorPlaySettings* PlayInSettings = GetDefault<ULevelEditorPlaySettings>();

	// We always need to create a new PIE world unless we're using the editor world for SIE
	UWorld* NewWorld = nullptr;

	bool bNeedsGarbageCollection = false;
	const EPlayNetMode PlayNetMode = [&PlayInSettings]{ EPlayNetMode NetMode(PIE_Standalone); return (PlayInSettings->GetPlayNetMode(NetMode) ? NetMode : PIE_Standalone); }();
	const bool CanRunUnderOneProcess = [&PlayInSettings]{ bool RunUnderOneProcess(false); return (PlayInSettings->GetRunUnderOneProcess(RunUnderOneProcess) && RunUnderOneProcess); }();
	if (PlayNetMode == PIE_Client)
	{
		// We are going to connect, so just load an empty world
		NewWorld = EditorEngine->CreatePIEWorldFromEntry(*WorldContext, EditorEngine->EditorWorld, PIEMapName);
	}
	else if (PlayNetMode == PIE_ListenServer && !CanRunUnderOneProcess)
	{
		// We *have* to save the world to disk in order to be a listen server that allows other processes to connect.
		// Otherwise, clients would not be able to load the world we are using
		NewWorld = EditorEngine->CreatePIEWorldBySavingToTemp(*WorldContext, EditorEngine->EditorWorld, PIEMapName);
	}
	else
	{
		// Standard PIE path: just duplicate the EditorWorld
		NewWorld = EditorEngine->CreatePIEWorldByDuplication(*WorldContext, EditorEngine->EditorWorld, PIEMapName);

		// Duplication can result in unreferenced objects, so indicate that we should do a GC pass after initializing the world context
		bNeedsGarbageCollection = true;
	}

	// failed to create the world!
	if (NewWorld == nullptr)
	{
		FMessageDialog::Open(EAppMsgType::Ok, NSLOCTEXT("UnrealEd", "Error_FailedCreateEditorPreviewWorld", "Failed to create editor preview world."));
		return false;
	}

	NewWorld->SetGameInstance(this);
	WorldContext->SetCurrentWorld(NewWorld);
	WorldContext->AddRef(EditorEngine->PlayWorld);	// Tie this context to this UEngine::PlayWorld*		// @fixme, needed still?

	// make sure we can clean up this world!
	NewWorld->ClearFlags(RF_Standalone);
	NewWorld->bKismetScriptError = bAnyBlueprintErrors;

	// Do a GC pass if necessary to remove any potentially unreferenced objects
	if(bNeedsGarbageCollection)
	{
		CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
	}

	Init();

	return true;
}

bool UGameInstance::StartPIEGameInstance(ULocalPlayer* LocalPlayer, bool bInSimulateInEditor, bool bAnyBlueprintErrors, bool bStartInSpectatorMode)
{
	UEditorEngine* const EditorEngine = CastChecked<UEditorEngine>(GetEngine());
	ULevelEditorPlaySettings const* PlayInSettings = GetDefault<ULevelEditorPlaySettings>();

	const EPlayNetMode PlayNetMode = [&PlayInSettings]{ EPlayNetMode NetMode(PIE_Standalone); return (PlayInSettings->GetPlayNetMode(NetMode) ? NetMode : PIE_Standalone); }();

	// for clients, just connect to the server
	if (PlayNetMode == PIE_Client)
	{
		FString Error;
		FURL BaseURL = WorldContext->LastURL;
		if (EditorEngine->Browse(*WorldContext, FURL(&BaseURL, TEXT("127.0.0.1"), (ETravelType)TRAVEL_Absolute), Error) == EBrowseReturnVal::Pending)
		{
			EditorEngine->TransitionType = TT_WaitingToConnect;
		}
		else
		{
			FMessageDialog::Open(EAppMsgType::Ok, FText::Format(NSLOCTEXT("UnrealEd", "Error_CouldntLaunchPIEClient", "Couldn't Launch PIE Client: {0}"), FText::FromString(Error)));
			return false;
		}
	}
	else
	{
		// we're going to be playing in the current world, get it ready for play
		UWorld* const PlayWorld = GetWorld();

		// make a URL
		FURL URL;
		// If the user wants to start in spectator mode, do not use the custom play world for now
		if (EditorEngine->UserEditedPlayWorldURL.Len() > 0 && !bStartInSpectatorMode)
		{
			// If the user edited the play world url. Verify that the map name is the same as the currently loaded map.
			URL = FURL(NULL, *EditorEngine->UserEditedPlayWorldURL, TRAVEL_Absolute);
			if (URL.Map != PIEMapName)
			{
				// Ensure the URL map name is the same as the generated play world map name.
				URL.Map = PIEMapName;
			}
		}
		else
		{
			// The user did not edit the url, just build one from scratch.
			URL = FURL(NULL, *EditorEngine->BuildPlayWorldURL(*PIEMapName, bStartInSpectatorMode), TRAVEL_Absolute);
		}

		// If a start location is specified, spawn a temporary PlayerStartPIE actor at the start location and use it as the portal.
		AActor* PlayerStart = NULL;
		if (EditorEngine->SpawnPlayFromHereStart(PlayWorld, PlayerStart, EditorEngine->PlayWorldLocation, EditorEngine->PlayWorldRotation) == false)
		{
			// failed to create "play from here" playerstart
			FMessageDialog::Open(EAppMsgType::Ok, NSLOCTEXT("UnrealEd", "Error_FailedCreatePlayFromHerePlayerStart", "Failed to create PlayerStart at desired starting location."));
			return false;
		}

		if (!PlayWorld->SetGameMode(URL))
		{
			// Setting the game mode failed so bail 
			FMessageDialog::Open(EAppMsgType::Ok, NSLOCTEXT("UnrealEd", "Error_FailedCreateEditorPreviewWorld", "Failed to create editor preview world."));
			return false;
		}
		
		// Make sure "always loaded" sub-levels are fully loaded
		PlayWorld->FlushLevelStreaming(EFlushLevelStreamingType::Visibility);

		UNavigationSystem::InitializeForWorld(PlayWorld, LocalPlayers.Num() > 0 ? FNavigationSystem::PIEMode : FNavigationSystem::SimulationMode);
		PlayWorld->CreateAISystem();

		PlayWorld->InitializeActorsForPlay(URL);

		// @todo, just use WorldContext.GamePlayer[0]?
		if (LocalPlayer)
		{
			FString Error;
			if (!LocalPlayer->SpawnPlayActor(URL.ToString(1), Error, PlayWorld))
			{
				FMessageDialog::Open(EAppMsgType::Ok, FText::Format(NSLOCTEXT("UnrealEd", "Error_CouldntSpawnPlayer", "Couldn't spawn player: {0}"), FText::FromString(Error)));
				return false;
			}
		}

		UGameViewportClient* const GameViewport = GetGameViewportClient();
		if (GameViewport != NULL && GameViewport->Viewport != NULL)
		{
			// Stream any levels now that need to be loaded before the game starts
			GEngine->BlockTillLevelStreamingCompleted(PlayWorld);
		}
		
		if (PlayNetMode == PIE_ListenServer)
		{
			// start listen server with the built URL
			PlayWorld->Listen(URL);
		}

		PlayWorld->BeginPlay();
	}

	return true;
}
#endif


UGameViewportClient* UGameInstance::GetGameViewportClient() const
{
	FWorldContext* const WC = GetWorldContext();
	return WC ? WC->GameViewport : nullptr;
}

void UGameInstance::StartGameInstance()
{
	UEngine* const Engine = GetEngine();

	// Create default URL.
	// @note: if we change how we determine the valid start up map update LaunchEngineLoop's GetStartupMap()
	FURL DefaultURL;
	DefaultURL.LoadURLConfig(TEXT("DefaultPlayer"), GGameIni);

	// Enter initial world.
	EBrowseReturnVal::Type BrowseRet = EBrowseReturnVal::Failure;
	FString Error;
	TCHAR Parm[4096] = TEXT("");
	const TCHAR* Tmp = FCommandLine::Get();

#if UE_BUILD_SHIPPING
	// In shipping don't allow an override
	Tmp = TEXT("");
#endif // UE_BUILD_SHIPPING

	const UGameMapsSettings* GameMapsSettings = GetDefault<UGameMapsSettings>();
	const FString& DefaultMap = GameMapsSettings->GetGameDefaultMap();
	if (!FParse::Token(Tmp, Parm, ARRAY_COUNT(Parm), 0) || Parm[0] == '-')
	{
		FCString::Strcpy(Parm, *(DefaultMap + GameMapsSettings->LocalMapOptions));
	}

	FURL URL(&DefaultURL, Parm, TRAVEL_Partial);
	if (URL.Valid)
	{
		BrowseRet = Engine->Browse(*WorldContext, URL, Error);
	}

	// If waiting for a network connection, go into the starting level.
	if (BrowseRet != EBrowseReturnVal::Success && FCString::Stricmp(Parm, *DefaultMap) != 0)
	{
		const FText Message = FText::Format(NSLOCTEXT("Engine", "MapNotFound", "The map specified on the commandline '{0}' could not be found. Would you like to load the default map instead?"), FText::FromString(URL.Map));

		// the map specified on the command-line couldn't be loaded.  ask the user if we should load the default map instead
		if (FCString::Stricmp(*URL.Map, *DefaultMap) != 0 &&
			FMessageDialog::Open(EAppMsgType::OkCancel, Message) != EAppReturnType::Ok)
		{
			// user canceled (maybe a typo while attempting to run a commandlet)
			FPlatformMisc::RequestExit(false);
			return;
		}
		else
		{
			BrowseRet = Engine->Browse(*WorldContext, FURL(&DefaultURL, *(DefaultMap + GameMapsSettings->LocalMapOptions), TRAVEL_Partial), Error);
		}
	}

	// Handle failure.
	if (BrowseRet != EBrowseReturnVal::Success)
	{
		UE_LOG(LogLoad, Fatal, TEXT("%s"), *FString::Printf(TEXT("Failed to enter %s: %s. Please check the log for errors."), Parm, *Error));
	}

}


bool UGameInstance::HandleOpenCommand(const TCHAR* Cmd, FOutputDevice& Ar, UWorld* InWorld)
{
	check(WorldContext && WorldContext->World() == InWorld);

	UEngine* const Engine = GetEngine();

	FURL TestURL(&WorldContext->LastURL, Cmd, TRAVEL_Absolute);
	if (TestURL.IsLocalInternal())
	{
		// make sure the file exists if we are opening a local file
		if (!Engine->MakeSureMapNameIsValid(TestURL.Map))
		{
			Ar.Logf(TEXT("ERROR: The map '%s' does not exist."), *TestURL.Map);
			return true;
		}
	}

	Engine->SetClientTravel(InWorld, Cmd, TRAVEL_Absolute);
	return true;
}

bool UGameInstance::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
	// @todo a bunch of stuff in UEngine probably belongs here as well
	if (FParse::Command(&Cmd, TEXT("OPEN")))
	{
		return HandleOpenCommand(Cmd, Ar, InWorld);
	}

	return false;
}

ULocalPlayer* UGameInstance::CreateInitialPlayer(FString& OutError)
{
	return CreateLocalPlayer( 0, OutError, false );
}

ULocalPlayer* UGameInstance::CreateLocalPlayer(int32 ControllerId, FString& OutError, bool bSpawnActor)
{
	check(GetEngine()->LocalPlayerClass != NULL);

	ULocalPlayer* NewPlayer = NULL;
	int32 InsertIndex = INDEX_NONE;

	const int32 MaxSplitscreenPlayers = (GetGameViewportClient() != NULL) ? GetGameViewportClient()->MaxSplitscreenPlayers : 1;

	if (FindLocalPlayerFromControllerId( ControllerId ) != NULL)
	{
		OutError = FString::Printf(TEXT("A local player already exists for controller ID %d,"), ControllerId);
	}
	else if (LocalPlayers.Num() < MaxSplitscreenPlayers)
	{
		// If the controller ID is not specified then find the first available
		if (ControllerId < 0)
		{
			for (ControllerId = 0; ControllerId < MaxSplitscreenPlayers; ++ControllerId)
			{
				if (FindLocalPlayerFromControllerId( ControllerId ) == NULL)
				{
					break;
				}
			}
			check(ControllerId < MaxSplitscreenPlayers);
		}
		else if (ControllerId >= MaxSplitscreenPlayers)
		{
			UE_LOG(LogPlayerManagement, Warning, TEXT("Controller ID (%d) is unlikely to map to any physical device, so this player will not receive input"), ControllerId);
		}

		NewPlayer = NewObject<ULocalPlayer>(GetEngine(), GetEngine()->LocalPlayerClass);
		InsertIndex = AddLocalPlayer(NewPlayer, ControllerId);
		if (bSpawnActor && InsertIndex != INDEX_NONE && GetWorld() != NULL)
		{
			if (GetWorld()->GetNetMode() != NM_Client)
			{
				// server; spawn a new PlayerController immediately
				if (!NewPlayer->SpawnPlayActor("", OutError, GetWorld()))
				{
					RemoveLocalPlayer(NewPlayer);
					NewPlayer = NULL;
				}
			}
			else
			{
				// client; ask the server to let the new player join
				NewPlayer->SendSplitJoin();
			}
		}
	}
	else
	{
		OutError = FString::Printf(TEXT( "Maximum number of players (%d) already created.  Unable to create more."), MaxSplitscreenPlayers);
	}

	if (OutError != TEXT(""))
	{
		UE_LOG(LogPlayerManagement, Log, TEXT("UPlayer* creation failed with error: %s"), *OutError);
	}

	return NewPlayer;
}

int32 UGameInstance::AddLocalPlayer(ULocalPlayer* NewLocalPlayer, int32 ControllerId)
{
	if (NewLocalPlayer == NULL)
	{
		return INDEX_NONE;
	}

	const int32 InsertIndex = LocalPlayers.Num();

	// Add to list
	LocalPlayers.AddUnique(NewLocalPlayer);

	// Notify the player he/she was added
	NewLocalPlayer->PlayerAdded(GetGameViewportClient(), ControllerId);

	// Notify the viewport that we added a player (so it can update splitscreen settings, etc)
	if ( GetGameViewportClient() != NULL )
	{
		GetGameViewportClient()->NotifyPlayerAdded(InsertIndex, NewLocalPlayer);
	}

	return InsertIndex;
}

bool UGameInstance::RemoveLocalPlayer(ULocalPlayer* ExistingPlayer)
{
	// FIXME: Notify server we want to leave the game if this is an online game
	if (ExistingPlayer->PlayerController != NULL)
	{
		// FIXME: Do this all inside PlayerRemoved?
		ExistingPlayer->PlayerController->CleanupGameViewport();

		// Destroy the player's actors.
		if ( ExistingPlayer->PlayerController->Role == ROLE_Authority )
		{
			ExistingPlayer->PlayerController->Destroy();
		}
	}

	// Remove the player from the context list
	const int32 OldIndex = LocalPlayers.Find(ExistingPlayer);

	if (ensure(OldIndex != INDEX_NONE))
	{
		ExistingPlayer->PlayerRemoved();
		LocalPlayers.RemoveAt(OldIndex);

		// Notify the viewport so the viewport can do the fixups, resize, etc
		if (GetGameViewportClient() != NULL)
		{
			GetGameViewportClient()->NotifyPlayerRemoved(OldIndex, ExistingPlayer);
		}
	}

	// Disassociate this viewport client from the player.
	// Do this after notifications, as some of them require the ViewportClient.
	ExistingPlayer->ViewportClient = NULL;

	UE_LOG(LogPlayerManagement, Log, TEXT("UGameInstance::RemovePlayer: Removed player %s with ControllerId %i at index %i (%i remaining players)"), *ExistingPlayer->GetName(), ExistingPlayer->GetControllerId(), OldIndex, LocalPlayers.Num());

	return true;
}

void UGameInstance::DebugCreatePlayer(int32 ControllerId)
{
#if !UE_BUILD_SHIPPING
	FString Error;
	CreateLocalPlayer(ControllerId, Error, true);
	if (Error.Len() > 0)
	{
		UE_LOG(LogPlayerManagement, Error, TEXT("Failed to DebugCreatePlayer: %s"), *Error);
	}
#endif
}

void UGameInstance::DebugRemovePlayer(int32 ControllerId)
{
#if !UE_BUILD_SHIPPING
	ULocalPlayer* const ExistingPlayer = FindLocalPlayerFromControllerId(ControllerId);
	if (ExistingPlayer != NULL)
	{
		RemoveLocalPlayer(ExistingPlayer);
	}
#endif
}

int32 UGameInstance::GetNumLocalPlayers() const
{
	return LocalPlayers.Num();
}

ULocalPlayer* UGameInstance::GetLocalPlayerByIndex(const int32 Index) const
{
	return LocalPlayers[Index];
}

APlayerController* UGameInstance::GetFirstLocalPlayerController() const
{
	for (ULocalPlayer* Player : LocalPlayers)
	{
		if (Player && Player->PlayerController)
		{
			// return first non-null entry
			return Player->PlayerController;
		}
	}

	// didn't find one
	return nullptr;
}

ULocalPlayer* UGameInstance::FindLocalPlayerFromControllerId(const int32 ControllerId) const
{
	for (ULocalPlayer * LP : LocalPlayers)
	{
		if (LP && (LP->GetControllerId() == ControllerId))
		{
			return LP;
		}
	}

	return nullptr;
}

ULocalPlayer* UGameInstance::FindLocalPlayerFromUniqueNetId(const FUniqueNetId& UniqueNetId) const
{
	for (ULocalPlayer* Player : LocalPlayers)
	{
		if (Player == NULL)
		{
			continue;
		}

		TSharedPtr<FUniqueNetId> OtherUniqueNetId = Player->GetPreferredUniqueNetId();

		if (!OtherUniqueNetId.IsValid())
		{
			continue;
		}

		if (*OtherUniqueNetId == UniqueNetId)
		{
			// Match
			return Player;
		}
	}

	// didn't find one
	return nullptr;
}

ULocalPlayer* UGameInstance::FindLocalPlayerFromUniqueNetId(TSharedPtr<FUniqueNetId> UniqueNetId) const
{
	if (!UniqueNetId.IsValid())
	{
		return nullptr;
	}

	return FindLocalPlayerFromUniqueNetId(*UniqueNetId);
}

ULocalPlayer* UGameInstance::GetFirstGamePlayer() const
{
	return (LocalPlayers.Num() > 0) ? LocalPlayers[0] : nullptr;
}

void UGameInstance::CleanupGameViewport()
{
	// Clean up the viewports that have been closed.
	for(int32 idx = LocalPlayers.Num()-1; idx >= 0; --idx)
	{
		ULocalPlayer *Player = LocalPlayers[idx];

		if(Player && Player->ViewportClient && !Player->ViewportClient->Viewport)
		{
			RemoveLocalPlayer( Player );
		}
	}
}

TArray<class ULocalPlayer*>::TConstIterator	UGameInstance::GetLocalPlayerIterator()
{
	return LocalPlayers.CreateConstIterator();
}

const TArray<class ULocalPlayer*>& UGameInstance::GetLocalPlayers()
{
	return LocalPlayers;
}

void UGameInstance::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector)
{
	UGameInstance* This = CastChecked<UGameInstance>(InThis);

	for (ULocalPlayer* LocalPlayer : This->LocalPlayers)
	{
		Collector.AddReferencedObject(LocalPlayer, This);
	}

	Super::AddReferencedObjects(This, Collector);
}

void UGameInstance::HandleSessionUserInviteAccepted(const bool bWasSuccess, const int32 ControllerId, TSharedPtr< FUniqueNetId > UserId, const FOnlineSessionSearchResult &	InviteResult)
{
	OnSessionUserInviteAccepted(bWasSuccess, ControllerId, UserId, InviteResult);
}

void UGameInstance::OnSessionUserInviteAccepted(const bool bWasSuccess, const int32 ControllerId, TSharedPtr< FUniqueNetId > UserId, const FOnlineSessionSearchResult &	InviteResult)
{
	UE_LOG(LogPlayerManagement, Verbose, TEXT("OnSessionUserInviteAccepted LocalUserNum: %d bSuccess: %d"), ControllerId, bWasSuccess);
	// Don't clear invite accept delegate

	if (bWasSuccess)
	{
		if (InviteResult.IsValid())
		{
			for (ULocalPlayer* LocalPlayer : LocalPlayers)
			{
				// Route the call to the actual user that accepted the invite
				if (LocalPlayer->GetCachedUniqueNetId() == UserId)
				{
					LocalPlayer->GetOnlineSession()->OnSessionUserInviteAccepted(bWasSuccess, ControllerId, UserId, InviteResult);
					return;
				}
			}

			// Go ahead and have the active local player handle accepting the invite. A game can detect that the user id is different and handle
			// it how it needs to.
			ULocalPlayer* LocalPlayer = GetFirstGamePlayer();
			if (LocalPlayer)
			{
				LocalPlayer->GetOnlineSession()->OnSessionUserInviteAccepted(bWasSuccess, ControllerId, UserId, InviteResult);
			}
		}
		else
		{
			UE_LOG(LogPlayerManagement, Warning, TEXT("Invite accept returned invalid search result."));
		}
	}
}

void UGameInstance::StartRecordingReplay(const FString& Name, const FString& FriendlyName)
{
	if ( FParse::Param( FCommandLine::Get(),TEXT( "NOREPLAYS" ) ) )
	{
		UE_LOG( LogDemo, Warning, TEXT( "UGameInstance::StartRecordingReplay: Rejected due to -noreplays option" ) );
		return;
	}

	UWorld* CurrentWorld = GetWorld();

	if ( CurrentWorld == nullptr )
	{
		UE_LOG( LogDemo, Warning, TEXT( "UGameInstance::StartRecordingReplay: GetWorld() is null" ) );
		return;
	}

	FURL DemoURL;
	FString DemoName = Name;
	
	DemoName.ReplaceInline( TEXT( "%m" ), *CurrentWorld->GetMapName() );

	// replace the current URL's map with a demo extension
	DemoURL.Map = DemoName;
	DemoURL.AddOption( *FString::Printf( TEXT( "DemoFriendlyName=%s" ), *FriendlyName ) );

	CurrentWorld->DestroyDemoNetDriver();

	const FName NAME_DemoNetDriver( TEXT( "DemoNetDriver" ) );

	if ( !GEngine->CreateNamedNetDriver( CurrentWorld, NAME_DemoNetDriver, NAME_DemoNetDriver ) )
	{
		UE_LOG( LogDemo, Warning, TEXT( "RecordReplay: failed to create demo net driver!" ) );
		return;
	}

	CurrentWorld->DemoNetDriver = Cast< UDemoNetDriver >( GEngine->FindNamedNetDriver( CurrentWorld, NAME_DemoNetDriver ) );

	check( CurrentWorld->DemoNetDriver != NULL );

	CurrentWorld->DemoNetDriver->SetWorld( CurrentWorld );

	FString Error;

	if ( !CurrentWorld->DemoNetDriver->InitListen( CurrentWorld, DemoURL, false, Error ) )
	{
		UE_LOG( LogDemo, Warning, TEXT( "Demo recording failed: %s" ), *Error );
		CurrentWorld->DemoNetDriver = NULL;
	}
	else
	{
		UE_LOG(LogDemo, Log, TEXT( "Num Network Actors: %i" ), CurrentWorld->NetworkActors.Num() );
	}
}

void UGameInstance::StopRecordingReplay()
{
	UWorld* CurrentWorld = GetWorld();

	if ( CurrentWorld == nullptr )
	{
		UE_LOG( LogDemo, Warning, TEXT( "UGameInstance::StopRecordingReplay: GetWorld() is null" ) );
		return;
	}

	CurrentWorld->DestroyDemoNetDriver();
}

void UGameInstance::PlayReplay(const FString& Name)
{
	UWorld* CurrentWorld = GetWorld();

	if ( CurrentWorld == nullptr )
	{
		UE_LOG( LogDemo, Warning, TEXT( "UGameInstance::PlayReplay: GetWorld() is null" ) );
		return;
	}

	CurrentWorld->DestroyDemoNetDriver();

	FURL DemoURL;
	UE_LOG( LogDemo, Log, TEXT( "PlayReplay: Attempting to play demo %s" ), *Name );

	DemoURL.Map = Name;

	const FName NAME_DemoNetDriver( TEXT( "DemoNetDriver" ) );

	if ( !GEngine->CreateNamedNetDriver( CurrentWorld, NAME_DemoNetDriver, NAME_DemoNetDriver ) )
	{
		UE_LOG(LogDemo, Warning, TEXT( "PlayReplay: failed to create demo net driver!" ) );
		return;
	}

	CurrentWorld->DemoNetDriver = Cast< UDemoNetDriver >( GEngine->FindNamedNetDriver( CurrentWorld, NAME_DemoNetDriver ) );

	check( CurrentWorld->DemoNetDriver != NULL );

	CurrentWorld->DemoNetDriver->SetWorld( CurrentWorld );

	FString Error;

	if ( !CurrentWorld->DemoNetDriver->InitConnect( CurrentWorld, DemoURL, Error ) )
	{
		UE_LOG(LogDemo, Warning, TEXT( "Demo playback failed: %s" ), *Error );
		CurrentWorld->DestroyDemoNetDriver();
	}
	else
	{
		FCoreUObjectDelegates::PostDemoPlay.Broadcast();
	}
}

void UGameInstance::AddUserToReplay(const FString& UserString)
{
	UWorld* CurrentWorld = GetWorld();

	if ( CurrentWorld != nullptr && CurrentWorld->DemoNetDriver != nullptr )
	{
		CurrentWorld->DemoNetDriver->AddUserToReplay( UserString );
	}
}

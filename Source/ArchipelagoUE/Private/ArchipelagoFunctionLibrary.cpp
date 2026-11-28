
// Copyright © 2026 Robin Hasenbach. All Rights Reserved.

#include "ArchipelagoFunctionLibrary.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"


// Subsystem Access

UArchipelagoSubsystem* UArchipelagoFunctionLibrary::GetArchipelagoSubsystem(const UObject* WorldContextObject)
{
    if (!GEngine || !IsValid(WorldContextObject))
        return nullptr;

    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);

    if (!World)
        return nullptr;

    UGameInstance* GameInstance = World->GetGameInstance();

    if (!GameInstance)
        return nullptr;

    return GameInstance->GetSubsystem<UArchipelagoSubsystem>();
}

#pragma region Setup

bool UArchipelagoFunctionLibrary::SetupArchipelago(
    const UObject* WorldContextObject,
    const FString& GameName,
    const FAPMessageTypeColorPalette& ColorPalette)
{
    UArchipelagoSubsystem* APSubsystem = GetArchipelagoSubsystem(WorldContextObject);

    if (!IsValid(APSubsystem))
        return false;

    return APSubsystem->SetupArchipelago(GameName, ColorPalette);
}

#pragma endregion

#pragma region Config

bool UArchipelagoFunctionLibrary::OverrideConfig(const UObject* WorldContextObject, const FAPConfig& NewConfig)
{
    UArchipelagoSubsystem* APSubsystem = GetArchipelagoSubsystem(WorldContextObject);

    if (!IsValid(APSubsystem))
        return false;

    return APSubsystem->OverrideConfig(NewConfig);
}

EAPConfigCreateResult UArchipelagoFunctionLibrary::CreateNewConfig(const UObject* WorldContextObject, bool bOverrideExisting)
{
    UArchipelagoSubsystem* APSubsystem = GetArchipelagoSubsystem(WorldContextObject);

    if (!IsValid(APSubsystem))
        return EAPConfigCreateResult::SubsystemNotAccessible;

    return APSubsystem->CreateNewConfig(bOverrideExisting);
}

EAPConfigLoadResult UArchipelagoFunctionLibrary::LoadConfig(const UObject* WorldContextObject)
{
    UArchipelagoSubsystem* APSubsystem = GetArchipelagoSubsystem(WorldContextObject);

    if (!IsValid(APSubsystem))
        return EAPConfigLoadResult::SubsystemNotAccessible;

    return APSubsystem->LoadConfig();
}

EAPConfigSaveResult UArchipelagoFunctionLibrary::SaveConfig(const UObject* WorldContextObject)
{
    UArchipelagoSubsystem* APSubsystem = GetArchipelagoSubsystem(WorldContextObject);

    if (!IsValid(APSubsystem))
        return EAPConfigSaveResult::SubsystemNotAccessible;

    return APSubsystem->SaveConfig();
}

#pragma endregion

#pragma region WebSocket

EAPWebSocketConnectionResult UArchipelagoFunctionLibrary::ConnectWebSocket(const UObject* WorldContextObject)
{
    UArchipelagoSubsystem* APSubsystem = GetArchipelagoSubsystem(WorldContextObject);

    if (!IsValid(APSubsystem))
        return EAPWebSocketConnectionResult::SubsystemNotAccessible;

    return APSubsystem->ConnectWebSocket();
}

EAPWebSocketDestroyResult UArchipelagoFunctionLibrary::DestroyWebSocket(const UObject* WorldContextObject)
{
    UArchipelagoSubsystem* APSubsystem = GetArchipelagoSubsystem(WorldContextObject);

    if (!IsValid(APSubsystem))
        return {};

    return APSubsystem->DestroyWebSocket();
}

#pragma endregion

#pragma region Locations

bool UArchipelagoFunctionLibrary::SendLocationCheck(const UObject* WorldContextObject, int64 LocationId)
{
    UArchipelagoSubsystem* APSubsystem = GetArchipelagoSubsystem(WorldContextObject);

    if (!IsValid(APSubsystem))
        return false;

    return APSubsystem->SendLocationCheck(LocationId);
}

bool UArchipelagoFunctionLibrary::SendLocationChecks(const UObject* WorldContextObject, const TArray<int64>& LocationIds)
{
    UArchipelagoSubsystem* APSubsystem = GetArchipelagoSubsystem(WorldContextObject);

    if (!IsValid(APSubsystem))
        return false;

    return APSubsystem->SendLocationChecks(LocationIds);
}

bool UArchipelagoFunctionLibrary::RequestLocationScout(const UObject* WorldContextObject, int64 LocationId)
{
    UArchipelagoSubsystem* APSubsystem = GetArchipelagoSubsystem(WorldContextObject);

    if (!IsValid(APSubsystem))
        return false;

    return APSubsystem->RequestLocationScout(LocationId);
}

bool UArchipelagoFunctionLibrary::RequestLocationScouts(const UObject* WorldContextObject, const TArray<int64>& LocationIds)
{
    UArchipelagoSubsystem* APSubsystem = GetArchipelagoSubsystem(WorldContextObject);

    if (!IsValid(APSubsystem))
        return false;

    return APSubsystem->RequestLocationScouts(LocationIds);
}

#pragma endregion

#pragma region Items

bool UArchipelagoFunctionLibrary::RequestSync(const UObject* WorldContextObject)
{
    UArchipelagoSubsystem* APSubsystem = GetArchipelagoSubsystem(WorldContextObject);

    if (!IsValid(APSubsystem))
        return false;

    return APSubsystem->RequestSync();
}

void UArchipelagoFunctionLibrary::ApplyItem(const UObject* WorldContextObject, int64 ItemID)
{
    UArchipelagoSubsystem* APSubsystem = GetArchipelagoSubsystem(WorldContextObject);

    if (!IsValid(APSubsystem))
        return;

    APSubsystem->ApplyItem(ItemID);
}

void UArchipelagoFunctionLibrary::MarkItemAsApplied(const UObject* WorldContextObject, int32 ItemIndex)
{
    UArchipelagoSubsystem* APSubsystem = GetArchipelagoSubsystem(WorldContextObject);

    if (!IsValid(APSubsystem))
        return;

    APSubsystem->MarkItemAsApplied(ItemIndex);
}

#pragma endregion

#pragma region Player

bool UArchipelagoFunctionLibrary::SendStatusUpdate(const UObject* WorldContextObject, EAPClientStatus Status)
{
    UArchipelagoSubsystem* APSubsystem = GetArchipelagoSubsystem(WorldContextObject);

    if (!IsValid(APSubsystem))
        return false;

    return APSubsystem->SendStatusUpdate(Status);
}

#pragma endregion

#pragma region Chat

bool UArchipelagoFunctionLibrary::SendChatMessage(const UObject* WorldContextObject, const FString& Message)
{
    UArchipelagoSubsystem* APSubsystem = GetArchipelagoSubsystem(WorldContextObject);

    if (!IsValid(APSubsystem))
        return false;

    return APSubsystem->SendChatMessage(Message);
}

#pragma endregion

#pragma region Save

bool UArchipelagoFunctionLibrary::SaveArchipelagoSave(const UObject* WorldContextObject, const FString& OriginalSavePath)
{
    UArchipelagoSubsystem* APSubsystem = GetArchipelagoSubsystem(WorldContextObject);

    if (!IsValid(APSubsystem))
        return false;

    return APSubsystem->SaveArchipelagoSave(OriginalSavePath);
}

bool UArchipelagoFunctionLibrary::LoadArchipelagoSave(const UObject* WorldContextObject, const FString& OriginalSavePath)
{
    UArchipelagoSubsystem* APSubsystem = GetArchipelagoSubsystem(WorldContextObject);

    if (!IsValid(APSubsystem))
        return false;

    return APSubsystem->LoadArchipelagoSave(OriginalSavePath);
}

#pragma endregion
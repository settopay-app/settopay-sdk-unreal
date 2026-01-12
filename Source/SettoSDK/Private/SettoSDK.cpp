// Copyright Setto. All Rights Reserved.

#include "SettoSDK.h"

#define LOCTEXT_NAMESPACE "FSettoSDKModule"

void FSettoSDKModule::StartupModule()
{
    UE_LOG(LogTemp, Log, TEXT("[SettoSDK] Module started"));
}

void FSettoSDKModule::ShutdownModule()
{
    UE_LOG(LogTemp, Log, TEXT("[SettoSDK] Module shutdown"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSettoSDKModule, SettoSDK)

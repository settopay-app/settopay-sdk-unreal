// Copyright Setto. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SettoSDKTypes.h"
#include "SettoSDKSubsystem.generated.h"

/**
 * Setto SDK Subsystem
 *
 * Main entry point for Setto payment integration.
 * Access via UGameplayStatics::GetGameInstance()->GetSubsystem<USettoSDKSubsystem>()
 */
UCLASS()
class SETTOSDK_API USettoSDKSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * Initialize the SDK with configuration
     * Must be called before OpenPayment
     */
    UFUNCTION(BlueprintCallable, Category = "Setto")
    void InitializeSDK(const FSettoConfig& Config);

    /**
     * Open payment flow
     * @param Params Payment parameters (amount, orderId)
     * @param OnComplete Callback when payment completes
     */
    UFUNCTION(BlueprintCallable, Category = "Setto")
    void OpenPayment(const FSettoPaymentParams& Params, const FOnSettoPaymentComplete& OnComplete);

    /**
     * Check if SDK is initialized
     */
    UFUNCTION(BlueprintPure, Category = "Setto")
    bool IsInitialized() const { return bIsInitialized; }

    /**
     * Reset SDK (for re-initialization)
     */
    UFUNCTION(BlueprintCallable, Category = "Setto")
    void Reset();

    /** Event fired when payment completes */
    UPROPERTY(BlueprintAssignable, Category = "Setto")
    FOnSettoPaymentCompleteMulticast OnPaymentComplete;

protected:
    /** Get base URL for current environment */
    FString GetBaseUrl() const;

    /** Request PaymentToken from server (Full Mode) */
    void RequestPaymentToken(const FSettoPaymentParams& Params);

    /** Open browser with URL */
    void OpenBrowser(const FString& Url);

    /** Handle URL scheme callback */
    void HandleUrlCallback(const FString& Url);

#if PLATFORM_ANDROID
    /** Set instance for Android JNI callback */
    static void SetAndroidCallbackInstance(USettoSDKSubsystem* Instance);
#endif

    /** Parse callback URL and extract result */
    FSettoPaymentResult ParseCallbackUrl(const FString& Url);

    /** Debug log helper */
    void DebugLog(const FString& Message) const;

private:
    bool bIsInitialized = false;
    FSettoConfig CurrentConfig;
    FOnSettoPaymentComplete PendingCallback;

    static const FString DevBaseUrl;
    static const FString ProdBaseUrl;
};

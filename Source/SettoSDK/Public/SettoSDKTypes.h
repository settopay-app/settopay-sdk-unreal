// Copyright Setto. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SettoSDKTypes.generated.h"

/**
 * SDK Environment
 */
UENUM(BlueprintType)
enum class ESettoEnvironment : uint8
{
    Dev UMETA(DisplayName = "Development"),
    Prod UMETA(DisplayName = "Production")
};

/**
 * Payment Result Status
 */
UENUM(BlueprintType)
enum class ESettoPaymentStatus : uint8
{
    Success UMETA(DisplayName = "Success"),
    Failed UMETA(DisplayName = "Failed"),
    Cancelled UMETA(DisplayName = "Cancelled")
};

/**
 * SDK Configuration
 */
USTRUCT(BlueprintType)
struct SETTOSDK_API FSettoConfig
{
    GENERATED_BODY()

    /** Environment (Dev or Prod) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setto")
    ESettoEnvironment Environment = ESettoEnvironment::Dev;

    /** IdP Token for auto-login (optional) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setto")
    FString IdpToken;

    /** Enable debug logging */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setto")
    bool bDebug = false;
};

/**
 * Payment Parameters
 */
USTRUCT(BlueprintType)
struct SETTOSDK_API FSettoPaymentParams
{
    GENERATED_BODY()

    /** Merchant ID (required) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setto")
    FString MerchantId;

    /** Payment amount in USD */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setto")
    FString Amount;

    /** Order ID (optional) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setto")
    FString OrderId;
};

/**
 * Payment Result
 */
USTRUCT(BlueprintType)
struct SETTOSDK_API FSettoPaymentResult
{
    GENERATED_BODY()

    /** Payment status */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setto")
    ESettoPaymentStatus Status = ESettoPaymentStatus::Cancelled;

    /** Payment ID (on success) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setto")
    FString PaymentId;

    /** Transaction hash (on success) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setto")
    FString TxHash;

    /** Error message (on failure) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setto")
    FString Error;
};

/** Delegate for payment completion */
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnSettoPaymentComplete, const FSettoPaymentResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSettoPaymentCompleteMulticast, const FSettoPaymentResult&, Result);

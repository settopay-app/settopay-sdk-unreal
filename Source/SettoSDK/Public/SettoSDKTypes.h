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

    /** IdP Token for auto-login (optional) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setto")
    FString IdpToken;
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

    /** Payer wallet address (returned from server) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setto")
    FString FromAddress;

    /** Settlement recipient address (returned from server) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setto")
    FString ToAddress;

    /** Payment amount in USD (e.g. "10.00", returned from server) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setto")
    FString Amount;

    /** Chain ID (e.g. 8453, 56, 900001, returned from server) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setto")
    int32 ChainId = 0;

    /** Token symbol (e.g. "USDC", "USDT", returned from server) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setto")
    FString TokenSymbol;

    /** Error message (on failure) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setto")
    FString Error;
};

/** Delegate for payment completion */
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnSettoPaymentComplete, const FSettoPaymentResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSettoPaymentCompleteMulticast, const FSettoPaymentResult&, Result);

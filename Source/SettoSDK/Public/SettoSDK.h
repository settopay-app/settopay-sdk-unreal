#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

// ============================================================================
// 환경 설정
// ============================================================================

/**
 * Setto SDK 환경 설정
 */
UENUM(BlueprintType)
enum class ESettoEnvironment : uint8
{
    /** 개발 환경 */
    Development,

    /** 프로덕션 환경 */
    Production
};

// ============================================================================
// 결제 결과
// ============================================================================

/**
 * 결제 상태
 */
UENUM(BlueprintType)
enum class EPaymentStatus : uint8
{
    Success,
    Failed,
    Cancelled
};

/**
 * 결제 결과
 */
USTRUCT(BlueprintType)
struct SETTOSDK_API FPaymentResult
{
    GENERATED_BODY()

    /** 결제 상태 */
    UPROPERTY(BlueprintReadOnly)
    EPaymentStatus Status = EPaymentStatus::Failed;

    /** 블록체인 트랜잭션 해시 (성공 시) */
    UPROPERTY(BlueprintReadOnly)
    FString TxId;

    /** Setto 결제 ID */
    UPROPERTY(BlueprintReadOnly)
    FString PaymentId;

    /** 에러 메시지 (실패 시) */
    UPROPERTY(BlueprintReadOnly)
    FString Error;
};

/**
 * 결제 요청 파라미터
 */
USTRUCT(BlueprintType)
struct SETTOSDK_API FPaymentParams
{
    GENERATED_BODY()

    /** 주문 ID */
    UPROPERTY(BlueprintReadWrite)
    FString OrderId;

    /** 결제 금액 */
    UPROPERTY(BlueprintReadWrite)
    float Amount = 0.0f;

    /** 통화 (기본: USD) */
    UPROPERTY(BlueprintReadWrite)
    FString Currency;
};

// ============================================================================
// SDK
// ============================================================================

/** 결제 완료 델리게이트 */
DECLARE_DELEGATE_OneParam(FOnPaymentComplete, const FPaymentResult& /* Result */);

/**
 * Setto Unreal SDK
 *
 * 시스템 브라우저를 사용하여 wallet.settopay.com과 연동합니다.
 */
class SETTOSDK_API FSettoSDK
{
public:
    /**
     * SDK 초기화
     * @param MerchantId 고객사 ID
     * @param Environment 환경 설정
     * @param ReturnScheme Custom URL Scheme
     */
    static void Initialize(
        const FString& MerchantId,
        ESettoEnvironment Environment,
        const FString& ReturnScheme);

    /**
     * 결제 창을 열고 결제를 진행합니다.
     * @param Params 결제 파라미터
     * @param OnComplete 결제 완료 콜백
     */
    static void OpenPayment(
        const FPaymentParams& Params,
        FOnPaymentComplete OnComplete);

    /**
     * Deep Link 처리
     * @param Url Deep Link URL
     */
    static void HandleDeepLink(const FString& Url);

private:
    static FString MerchantId;
    static FString ReturnScheme;
    static ESettoEnvironment Environment;
    static FOnPaymentComplete OnPaymentComplete;

    static FString GetBaseUrl();
    static EPaymentStatus ParseStatus(const FString& StatusString);
};

/**
 * Setto SDK 모듈
 */
class FSettoSDKModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};

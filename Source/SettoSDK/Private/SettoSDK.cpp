#include "SettoSDK.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "GenericPlatform/GenericPlatformProcess.h"

// 정적 멤버 초기화
FString FSettoSDK::MerchantId;
FString FSettoSDK::ReturnScheme;
ESettoEnvironment FSettoSDK::Environment = ESettoEnvironment::Production;
FOnPaymentComplete FSettoSDK::OnPaymentComplete;

void FSettoSDK::Initialize(
    const FString& InMerchantId,
    ESettoEnvironment InEnvironment,
    const FString& InReturnScheme)
{
    MerchantId = InMerchantId;
    Environment = InEnvironment;
    ReturnScheme = InReturnScheme;
}

void FSettoSDK::OpenPayment(
    const FPaymentParams& Params,
    FOnPaymentComplete OnComplete)
{
    OnPaymentComplete = OnComplete;

    // URL 인코딩
    FString EncodedMerchantId = FGenericPlatformHttp::UrlEncode(MerchantId);
    FString EncodedOrderId = FGenericPlatformHttp::UrlEncode(Params.OrderId);
    FString EncodedScheme = FGenericPlatformHttp::UrlEncode(ReturnScheme);

    // URL 생성
    FString Url = FString::Printf(
        TEXT("%s/pay?merchantId=%s&orderId=%s&amount=%.2f&returnScheme=%s"),
        *GetBaseUrl(),
        *EncodedMerchantId,
        *EncodedOrderId,
        Params.Amount,
        *EncodedScheme);

    // 통화 추가
    if (!Params.Currency.IsEmpty())
    {
        FString EncodedCurrency = FGenericPlatformHttp::UrlEncode(Params.Currency);
        Url += FString::Printf(TEXT("&currency=%s"), *EncodedCurrency);
    }

    // 시스템 브라우저 열기
    FPlatformProcess::LaunchURL(*Url, nullptr, nullptr);
}

void FSettoSDK::HandleDeepLink(const FString& Url)
{
    // URL 파싱
    int32 QueryStart = Url.Find(TEXT("?"));
    if (QueryStart == INDEX_NONE)
    {
        FPaymentResult Result;
        Result.Status = EPaymentStatus::Failed;
        Result.Error = TEXT("Invalid URL");
        OnPaymentComplete.ExecuteIfBound(Result);
        return;
    }

    FString Query = Url.RightChop(QueryStart + 1);
    TMap<FString, FString> Params;

    // 쿼리 파라미터 파싱
    TArray<FString> Pairs;
    Query.ParseIntoArray(Pairs, TEXT("&"));

    for (const FString& Pair : Pairs)
    {
        FString Key, Value;
        if (Pair.Split(TEXT("="), &Key, &Value))
        {
            // URL 디코딩
            Params.Add(Key, FGenericPlatformHttp::UrlDecode(Value));
        }
    }

    // 결과 생성
    FPaymentResult Result;
    Result.Status = ParseStatus(Params.FindRef(TEXT("status")));
    Result.TxId = Params.FindRef(TEXT("txId"));
    Result.PaymentId = Params.FindRef(TEXT("paymentId"));
    Result.Error = Params.FindRef(TEXT("error"));

    // 콜백 호출
    OnPaymentComplete.ExecuteIfBound(Result);
}

FString FSettoSDK::GetBaseUrl()
{
    switch (Environment)
    {
    case ESettoEnvironment::Development:
        return TEXT("https://dev-wallet.settopay.com");
    case ESettoEnvironment::Production:
    default:
        return TEXT("https://wallet.settopay.com");
    }
}

EPaymentStatus FSettoSDK::ParseStatus(const FString& StatusString)
{
    if (StatusString == TEXT("success"))
        return EPaymentStatus::Success;
    if (StatusString == TEXT("cancelled"))
        return EPaymentStatus::Cancelled;
    return EPaymentStatus::Failed;
}

// 모듈 구현
void FSettoSDKModule::StartupModule()
{
    // 모듈 시작 시 초기화 로직
}

void FSettoSDKModule::ShutdownModule()
{
    // 모듈 종료 시 정리 로직
}

IMPLEMENT_MODULE(FSettoSDKModule, SettoSDK)

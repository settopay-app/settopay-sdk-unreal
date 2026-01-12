// Copyright Setto. All Rights Reserved.

#include "SettoSDKSubsystem.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/App.h"

#if PLATFORM_IOS
#include "IOS/IOSAppDelegate.h"
#import <SafariServices/SafariServices.h>
#endif

#if PLATFORM_ANDROID
#include "Android/AndroidJNI.h"
#include "Android/AndroidApplication.h"
#endif

const FString USettoSDKSubsystem::DevBaseUrl = TEXT("https://dev-wallet.settopay.com");
const FString USettoSDKSubsystem::ProdBaseUrl = TEXT("https://wallet.settopay.com");

void USettoSDKSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("[SettoSDK] Subsystem initialized"));
}

void USettoSDKSubsystem::Deinitialize()
{
    Reset();
    Super::Deinitialize();
}

void USettoSDKSubsystem::InitializeSDK(const FSettoConfig& Config)
{
    if (bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("[SettoSDK] Already initialized"));
        return;
    }

    if (Config.MerchantId.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("[SettoSDK] MerchantId is required"));
        return;
    }

    CurrentConfig = Config;
    bIsInitialized = true;

    DebugLog(FString::Printf(TEXT("Initialized with MerchantId: %s, Env: %s"),
        *Config.MerchantId,
        Config.Environment == ESettoEnvironment::Dev ? TEXT("Dev") : TEXT("Prod")));
}

void USettoSDKSubsystem::OpenPayment(const FSettoPaymentParams& Params, const FOnSettoPaymentComplete& OnComplete)
{
    if (!bIsInitialized)
    {
        UE_LOG(LogTemp, Error, TEXT("[SettoSDK] Not initialized. Call InitializeSDK first."));
        FSettoPaymentResult Result;
        Result.Status = ESettoPaymentStatus::Failed;
        Result.Error = TEXT("SDK not initialized");
        OnComplete.ExecuteIfBound(Result);
        return;
    }

    if (Params.Amount.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("[SettoSDK] Amount is required"));
        FSettoPaymentResult Result;
        Result.Status = ESettoPaymentStatus::Failed;
        Result.Error = TEXT("Amount is required");
        OnComplete.ExecuteIfBound(Result);
        return;
    }

    PendingCallback = OnComplete;

    if (CurrentConfig.IdpToken.IsEmpty())
    {
        // Simple Mode: Direct URL with query params
        FString Url = FString::Printf(TEXT("%s/pay/wallet?merchant_id=%s&amount=%s"),
            *GetBaseUrl(),
            *CurrentConfig.MerchantId,
            *Params.Amount);

        if (!Params.OrderId.IsEmpty())
        {
            Url += FString::Printf(TEXT("&order_id=%s"), *Params.OrderId);
        }

        DebugLog(FString::Printf(TEXT("Opening Simple Mode: %s"), *Url));
        OpenBrowser(Url);
    }
    else
    {
        // Full Mode: Request PaymentToken first
        DebugLog(TEXT("Requesting PaymentToken for Full Mode"));
        RequestPaymentToken(Params);
    }
}

void USettoSDKSubsystem::Reset()
{
    bIsInitialized = false;
    CurrentConfig = FSettoConfig();
    PendingCallback.Clear();
    DebugLog(TEXT("SDK Reset"));
}

FString USettoSDKSubsystem::GetBaseUrl() const
{
    return CurrentConfig.Environment == ESettoEnvironment::Dev ? DevBaseUrl : ProdBaseUrl;
}

void USettoSDKSubsystem::RequestPaymentToken(const FSettoPaymentParams& Params)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

    Request->SetURL(GetBaseUrl() + TEXT("/api/external/payment/token"));
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

    // Build JSON body
    TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();
    JsonObject->SetStringField(TEXT("merchant_id"), CurrentConfig.MerchantId);
    JsonObject->SetStringField(TEXT("amount"), Params.Amount);
    if (!Params.OrderId.IsEmpty())
    {
        JsonObject->SetStringField(TEXT("order_id"), Params.OrderId);
    }
    JsonObject->SetStringField(TEXT("idp_token"), CurrentConfig.IdpToken);

    FString JsonBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonBody);
    FJsonSerializer::Serialize(JsonObject, Writer);

    Request->SetContentAsString(JsonBody);

    Request->OnProcessRequestComplete().BindLambda(
        [this](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
        {
            if (!bSuccess || !Response.IsValid())
            {
                DebugLog(TEXT("PaymentToken request failed: Network error"));
                FSettoPaymentResult Result;
                Result.Status = ESettoPaymentStatus::Failed;
                Result.Error = TEXT("Network error");
                PendingCallback.ExecuteIfBound(Result);
                OnPaymentComplete.Broadcast(Result);
                return;
            }

            FString ResponseBody = Response->GetContentAsString();
            DebugLog(FString::Printf(TEXT("PaymentToken response: %s"), *ResponseBody));

            TSharedPtr<FJsonObject> JsonResponse;
            TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);
            if (!FJsonSerializer::Deserialize(Reader, JsonResponse) || !JsonResponse.IsValid())
            {
                FSettoPaymentResult Result;
                Result.Status = ESettoPaymentStatus::Failed;
                Result.Error = TEXT("Invalid response");
                PendingCallback.ExecuteIfBound(Result);
                OnPaymentComplete.Broadcast(Result);
                return;
            }

            // Check for errors
            if (JsonResponse->HasField(TEXT("payment_error")) || JsonResponse->HasField(TEXT("system_error")))
            {
                FString Error = JsonResponse->HasField(TEXT("payment_error"))
                    ? JsonResponse->GetStringField(TEXT("payment_error"))
                    : JsonResponse->GetStringField(TEXT("system_error"));

                FSettoPaymentResult Result;
                Result.Status = ESettoPaymentStatus::Failed;
                Result.Error = Error;
                PendingCallback.ExecuteIfBound(Result);
                OnPaymentComplete.Broadcast(Result);
                return;
            }

            // Get PaymentToken
            FString PaymentToken;
            if (!JsonResponse->TryGetStringField(TEXT("payment_token"), PaymentToken))
            {
                FSettoPaymentResult Result;
                Result.Status = ESettoPaymentStatus::Failed;
                Result.Error = TEXT("Payment token not received");
                PendingCallback.ExecuteIfBound(Result);
                OnPaymentComplete.Broadcast(Result);
                return;
            }

            // Open browser with PaymentToken (Fragment for security)
            FString Url = FString::Printf(TEXT("%s/pay/wallet#pt=%s"),
                *GetBaseUrl(),
                *FGenericPlatformHttp::UrlEncode(PaymentToken));

            DebugLog(FString::Printf(TEXT("Opening Full Mode: %s"), *Url));
            OpenBrowser(Url);
        });

    Request->ProcessRequest();
}

void USettoSDKSubsystem::OpenBrowser(const FString& Url)
{
#if PLATFORM_IOS
    // iOS: Use SFSafariViewController
    dispatch_async(dispatch_get_main_queue(), ^{
        NSURL* nsUrl = [NSURL URLWithString:Url.GetNSString()];
        SFSafariViewController* safari = [[SFSafariViewController alloc] initWithURL:nsUrl];
        safari.modalPresentationStyle = UIModalPresentationPageSheet;

        UIViewController* rootVC = [IOSAppDelegate GetDelegate].IOSController;
        [rootVC presentViewController:safari animated:YES completion:nil];
    });
#elif PLATFORM_ANDROID
    // Android: Use Chrome Custom Tabs via JNI
    if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
    {
        jstring jUrl = Env->NewStringUTF(TCHAR_TO_UTF8(*Url));
        jclass SettoClass = FAndroidApplication::FindJavaClass("com/setto/sdk/SettoSDK");
        if (SettoClass)
        {
            jmethodID OpenBrowserMethod = Env->GetStaticMethodID(SettoClass, "openBrowser", "(Landroid/app/Activity;Ljava/lang/String;)V");
            if (OpenBrowserMethod)
            {
                Env->CallStaticVoidMethod(SettoClass, OpenBrowserMethod, FAndroidApplication::GetGameActivityThis(), jUrl);
            }
        }
        Env->DeleteLocalRef(jUrl);
    }
#else
    // Desktop: Use system default browser
    FPlatformProcess::LaunchURL(*Url, nullptr, nullptr);
#endif

    DebugLog(FString::Printf(TEXT("Browser opened: %s"), *Url));
}

void USettoSDKSubsystem::HandleUrlCallback(const FString& Url)
{
    DebugLog(FString::Printf(TEXT("URL Callback received: %s"), *Url));

    FSettoPaymentResult Result = ParseCallbackUrl(Url);

    PendingCallback.ExecuteIfBound(Result);
    OnPaymentComplete.Broadcast(Result);
    PendingCallback.Clear();
}

FSettoPaymentResult USettoSDKSubsystem::ParseCallbackUrl(const FString& Url)
{
    FSettoPaymentResult Result;

    // Parse URL: setto-{merchantId}://callback?status=success&payment_id=xxx&tx_hash=xxx
    // Or: setto-{merchantId}://callback?status=failed&error=xxx
    // Or: setto-{merchantId}://callback?status=cancelled

    FString QueryString;
    if (Url.Split(TEXT("?"), nullptr, &QueryString))
    {
        TMap<FString, FString> Params;
        TArray<FString> Pairs;
        QueryString.ParseIntoArray(Pairs, TEXT("&"));

        for (const FString& Pair : Pairs)
        {
            FString Key, Value;
            if (Pair.Split(TEXT("="), &Key, &Value))
            {
                Params.Add(Key, FGenericPlatformHttp::UrlDecode(Value));
            }
        }

        FString Status = Params.FindRef(TEXT("status"));
        if (Status == TEXT("success"))
        {
            Result.Status = ESettoPaymentStatus::Success;
            Result.PaymentId = Params.FindRef(TEXT("payment_id"));
            Result.TxHash = Params.FindRef(TEXT("tx_hash"));
        }
        else if (Status == TEXT("failed"))
        {
            Result.Status = ESettoPaymentStatus::Failed;
            Result.Error = Params.FindRef(TEXT("error"));
        }
        else
        {
            Result.Status = ESettoPaymentStatus::Cancelled;
        }
    }
    else
    {
        Result.Status = ESettoPaymentStatus::Cancelled;
    }

    return Result;
}

void USettoSDKSubsystem::DebugLog(const FString& Message) const
{
    if (CurrentConfig.bDebug)
    {
        UE_LOG(LogTemp, Log, TEXT("[SettoSDK] %s"), *Message);
    }
}

// ============================================
// Android JNI Native Callback
// ============================================

#if PLATFORM_ANDROID
// Static instance pointer for JNI callback
static USettoSDKSubsystem* GSettoSDKInstance = nullptr;

void USettoSDKSubsystem::SetAndroidCallbackInstance(USettoSDKSubsystem* Instance)
{
    GSettoSDKInstance = Instance;
}

extern "C"
{
    JNIEXPORT void JNICALL Java_com_setto_sdk_SettoCallbackActivity_nativeOnPaymentCallback(JNIEnv* Env, jclass Class, jstring JUrl)
    {
        if (GSettoSDKInstance && JUrl)
        {
            const char* UrlChars = Env->GetStringUTFChars(JUrl, nullptr);
            FString Url = UTF8_TO_TCHAR(UrlChars);
            Env->ReleaseStringUTFChars(JUrl, UrlChars);

            // Execute on game thread
            AsyncTask(ENamedThreads::GameThread, [Url]()
            {
                if (GSettoSDKInstance)
                {
                    GSettoSDKInstance->HandleUrlCallback(Url);
                }
            });
        }
    }
}
#endif

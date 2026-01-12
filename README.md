# Setto SDK for Unreal Engine

Setto Unreal SDK - 시스템 브라우저 기반 결제 연동 SDK

## 요구사항

- Unreal Engine 4.27+ 또는 5.0+

## 지원 플랫폼

| 플랫폼 | 지원 |
|--------|------|
| Windows | ✅ |
| Mac | ✅ |
| iOS | ✅ |
| Android | ✅ |

## 설치

### GitHub Releases

1. [Releases](https://github.com/settopay-app/setto-unreal-sdk/releases)에서 최신 버전 다운로드
2. 압축 해제 후 `Plugins/SettoSDK` 폴더에 복사
3. 프로젝트 재시작

### Git Submodule

```bash
cd YourProject/Plugins
git submodule add https://github.com/settopay-app/setto-unreal-sdk.git SettoSDK
```

## 설정

### 모바일 - Custom URL Scheme

#### iOS

Project Settings → Platforms → iOS → Additional Plist Data:
```xml
<key>CFBundleURLTypes</key>
<array>
    <dict>
        <key>CFBundleURLSchemes</key>
        <array>
            <string>mygame</string>
        </array>
    </dict>
</array>
```

#### Android

Project Settings → Platforms → Android → Extra Settings for `<activity>`:
```xml
<intent-filter>
    <action android:name="android.intent.action.VIEW" />
    <category android:name="android.intent.category.DEFAULT" />
    <category android:name="android.intent.category.BROWSABLE" />
    <data android:scheme="mygame" />
</intent-filter>
```

### Deep Link 수신

플랫폼별 Deep Link 수신 방법은 [Multi-Platform 기술 가이드](../../docs/multi_platform_details.md)를 참조하세요.

## 사용법

### Build.cs 설정

```csharp
// YourGame.Build.cs
PublicDependencyModuleNames.AddRange(new string[] { "SettoSDK" });
```

### C++ 코드

```cpp
#include "SettoSDK.h"

// SDK 초기화 (게임 시작 시)
void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();

    FSettoSDK::Initialize(
        TEXT("your-merchant-id"),
        ESettoEnvironment::Production,
        TEXT("mygame")
    );
}

// 결제 요청
void AMyPlayerController::HandlePayment()
{
    FPaymentParams Params;
    Params.OrderId = TEXT("order-123");
    Params.Amount = 100.0f;
    Params.Currency = TEXT("USD");

    FSettoSDK::OpenPayment(Params, FOnPaymentComplete::CreateLambda(
        [](const FPaymentResult& Result)
        {
            switch (Result.Status)
            {
            case EPaymentStatus::Success:
                UE_LOG(LogTemp, Log, TEXT("결제 성공! TX ID: %s"), *Result.TxId);
                // 서버에서 결제 검증 필수!
                break;

            case EPaymentStatus::Cancelled:
                UE_LOG(LogTemp, Log, TEXT("사용자가 결제를 취소했습니다."));
                break;

            case EPaymentStatus::Failed:
                UE_LOG(LogTemp, Error, TEXT("결제 실패: %s"), *Result.Error);
                break;
            }
        }
    ));
}

// Deep Link 처리 (플랫폼별 수신 후 호출)
void AMyGameMode::OnDeepLinkReceived(const FString& Url)
{
    FSettoSDK::HandleDeepLink(Url);
}
```

### Blueprint

Blueprint에서 사용하려면 UObject 래퍼 클래스를 만들거나,
프로젝트의 C++ 코드에서 Blueprint 함수로 노출하세요.

## API

### FSettoSDK

#### `Initialize(MerchantId, Environment, ReturnScheme)`

SDK를 초기화합니다.

| 파라미터 | 타입 | 설명 |
|---------|------|------|
| `MerchantId` | `FString` | 고객사 ID |
| `Environment` | `ESettoEnvironment` | `Development` 또는 `Production` |
| `ReturnScheme` | `FString` | Custom URL Scheme |

#### `OpenPayment(Params, OnComplete)`

결제 창을 열고 결제를 진행합니다.

| 파라미터 | 타입 | 설명 |
|---------|------|------|
| `Params` | `FPaymentParams` | 결제 파라미터 |
| `OnComplete` | `FOnPaymentComplete` | 결제 완료 콜백 |

#### `HandleDeepLink(Url)`

Deep Link를 처리합니다.

### FPaymentParams

| 속성 | 타입 | 필수 | 설명 |
|------|------|------|------|
| `OrderId` | `FString` | ✅ | 주문 ID |
| `Amount` | `float` | ✅ | 결제 금액 |
| `Currency` | `FString` | | 통화 (기본: USD) |

### FPaymentResult

| 속성 | 타입 | 설명 |
|------|------|------|
| `Status` | `EPaymentStatus` | `Success`, `Failed`, `Cancelled` |
| `TxId` | `FString` | 블록체인 트랜잭션 해시 |
| `PaymentId` | `FString` | Setto 결제 ID |
| `Error` | `FString` | 에러 메시지 |

## 보안 참고사항

1. **결제 결과는 서버에서 검증 필수**: SDK에서 반환하는 결과는 UX 피드백용입니다. 실제 결제 완료 여부는 고객사 서버에서 Setto API를 통해 검증해야 합니다.

2. **Custom URL Scheme 보안**: 다른 앱이 동일한 Scheme을 등록할 수 있으므로, 결제 결과는 반드시 서버에서 검증하세요.

## License

MIT

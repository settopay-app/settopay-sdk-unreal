# Setto SDK for Unreal Engine

Unreal Engine plugin for integrating Setto payments.

## Installation

1. Copy the `SettoSDK` folder to your project's `Plugins` directory
2. Enable the plugin in your `.uproject` file or via Editor
3. Regenerate project files

## Quick Start

### Blueprint

```
// 1. Get SDK Subsystem
Get Game Instance -> Get Subsystem (SettoSDKSubsystem)

// 2. Initialize
Call InitializeSDK with:
  - MerchantId: "your-merchant-id"
  - Environment: Dev or Prod
  - IdpToken: (optional) for auto-login
  - bDebug: true

// 3. Open Payment
Call OpenPayment with:
  - Amount: "10.00"
  - OrderId: (optional)
Bind OnComplete delegate
```

### C++

```cpp
#include "SettoSDKSubsystem.h"

// Get subsystem
USettoSDKSubsystem* SettoSDK = GetGameInstance()->GetSubsystem<USettoSDKSubsystem>();

// Initialize
FSettoConfig Config;
Config.MerchantId = TEXT("your-merchant-id");
Config.Environment = ESettoEnvironment::Dev;
Config.bDebug = true;
SettoSDK->InitializeSDK(Config);

// Open payment
FSettoPaymentParams Params;
Params.Amount = TEXT("10.00");

SettoSDK->OpenPayment(Params, FOnSettoPaymentComplete::CreateLambda(
    [](const FSettoPaymentResult& Result)
    {
        if (Result.Status == ESettoPaymentStatus::Success)
        {
            UE_LOG(LogTemp, Log, TEXT("Payment Success: %s"), *Result.PaymentId);
        }
    }
));
```

## Platform Notes

### iOS
- Uses SFSafariViewController
- Add URL scheme to Info.plist: `setto-{merchantId}`

### Android
- Uses Chrome Custom Tabs
- URL scheme handled automatically via UPL

### Desktop (Windows/Mac/Linux)
- Opens system default browser
- Callback via URL scheme requires app registration

## API Reference

### FSettoConfig

| Property | Type | Description |
|----------|------|-------------|
| MerchantId | FString | Required. Your merchant ID |
| Environment | ESettoEnvironment | Dev or Prod |
| IdpToken | FString | Optional. For auto-login |
| bDebug | bool | Enable debug logging |

### FSettoPaymentParams

| Property | Type | Description |
|----------|------|-------------|
| Amount | FString | Required. Payment amount in USD |
| OrderId | FString | Optional. Your order reference |

### FSettoPaymentResult

| Property | Type | Description |
|----------|------|-------------|
| Status | ESettoPaymentStatus | Success, Failed, or Cancelled |
| PaymentId | FString | Payment ID (on success) |
| TxHash | FString | Transaction hash (on success) |
| Error | FString | Error message (on failure) |

## Requirements

- Unreal Engine 5.0+
- iOS 13+ / Android API 21+

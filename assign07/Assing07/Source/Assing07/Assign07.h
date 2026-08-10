#pragma once

#include "CoreMinimal.h"

#pragma region NetLogging

ASSING07_API DECLARE_LOG_CATEGORY_EXTERN(LogAssign07Net, Log, All);

#define NETMODE_TCHAR \
	((GetNetMode() == ENetMode::NM_Client) \
		? *FString::Printf(TEXT("Client%02d"), UE::GetPlayInEditorID()) \
		: ((GetNetMode() == ENetMode::NM_Standalone) \
			? TEXT("Standalone") \
			: TEXT("Server")))

#define FUNCTION_TCHAR ANSI_TO_TCHAR(__FUNCTION__)

#define ASSIGN07_LOG_NET(LogCategory, Verbosity, Format, ...) \
	UE_LOG( \
		LogCategory, \
		Verbosity, \
		TEXT("[%s] %s %s"), \
		NETMODE_TCHAR, \
		FUNCTION_TCHAR, \
		*FString::Printf(Format, ##__VA_ARGS__))

#define LOCAL_ROLE_TCHAR \
	*UEnum::GetValueAsString(TEXT("Engine.ENetRole"), GetLocalRole())

#define REMOTE_ROLE_TCHAR \
	*UEnum::GetValueAsString(TEXT("Engine.ENetRole"), GetRemoteRole())

#define ASSIGN07_LOG_ROLE(LogCategory, Verbosity, Format, ...) \
	UE_LOG( \
		LogCategory, \
		Verbosity, \
		TEXT("[%s][%s/%s] %s %s"), \
		NETMODE_TCHAR, \
		LOCAL_ROLE_TCHAR, \
		REMOTE_ROLE_TCHAR, \
		FUNCTION_TCHAR, \
		*FString::Printf(Format, ##__VA_ARGS__))

#pragma endregion
#include "Temporary.h"

#define LOCTEXT_NAMESPACE "FTemporaryModule"

void FTemporaryModule::StartupModule()
{
	// 플러그인이 정상적으로 로드되었는지 Output Log에서 확인할 수 있습니다.
	UE_LOG(LogTemp, Display, TEXT("Temporary plugin module started."));
}

void FTemporaryModule::ShutdownModule()
{
	UE_LOG(LogTemp, Display, TEXT("Temporary plugin module shut down."));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FTemporaryModule, Temporary);

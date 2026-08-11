#include "Test.h"

#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogTestModule);

// 독립된 일반 런타임 모듈을 언리얼의 모듈 관리자에 등록합니다.
IMPLEMENT_MODULE(FDefaultModuleImpl, Test);

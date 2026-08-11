# Assing07 - Module and Plugin

언리얼 엔진의 모듈과 플러그인 구조를 연습하기 위한 과제 프로젝트입니다.

## 구현 내용

### Test 런타임 모듈

- `Source/Test`에 독립된 런타임 모듈을 구성했습니다.
- `.uproject`에서는 `PreDefault` 단계에 로드합니다.
- `ATestActor`는 시작할 때 Output Log에 생성 성공 메시지를 출력합니다.
- 기본 게임 모듈의 `AAssign07Character::BeginPlay()`가 서버 권한에서 `ATestActor`를 생성합니다.
- 기본 게임 모듈이 Test 모듈의 클래스를 참조하도록 `Assing07.Build.cs`에 의존성을 등록했습니다.

### Temporary 런타임 플러그인

- `Plugins/Temporary`에 콘텐츠를 포함할 수 있는 런타임 플러그인을 구성했습니다.
- `StartupModule()`과 `ShutdownModule()`에서 각각 시작 및 종료 로그를 출력합니다.
- `.uproject`에서 플러그인을 활성화했습니다.

## 확인 방법

1. `Assing07.uproject`에서 Visual Studio/Rider 프로젝트 파일을 다시 생성합니다.
2. `Assing07Editor` 타깃을 Development Editor 구성으로 빌드합니다.
3. 에디터를 실행하고 플레이합니다.
4. Output Log에서 다음 메시지를 확인합니다.
   - `Temporary plugin module started.`
   - `TestActor was spawned successfully by the Assing07 module.`
5. 플러그인 창에서 `Temporary`가 활성화됐는지 확인합니다.
6. 콘텐츠 브라우저 설정에서 플러그인 콘텐츠 표시를 켜고 Temporary 폴더를 확인합니다.

멀티플레이 PIE에서는 TestActor의 중복 생성을 막기 위해 서버 권한을 가진 캐릭터만 액터를 생성합니다.

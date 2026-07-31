# Home Run! 숫자 야구

React, TypeScript, SpaceTimeDB로 만든 실시간 멀티플레이 숫자 야구 게임입니다.

## 구현 기능

- 서버에서 1~9 사이의 중복 없는 정답 3자리 생성
- 서버 권한 기반 입력 검증과 S/B/OUT 판정
- 플레이어별 3회 기회 및 접속 상태 관리
- 승리, 무승부, 다음 라운드 처리
- 플레이어 이름 변경
- 접속자와 판정 기록 실시간 동기화
- 반응형 게임 UI

정답을 저장하는 `game_secret`은 private 테이블입니다. 클라이언트에는
`player`, `game_state`, `guess_result` 테이블만 공개됩니다.

## 로컬 실행

SpaceTimeDB CLI와 Node.js가 필요합니다.

터미널 1:

```bash
spacetime start
```

터미널 2:

```bash
cd ch-4
spacetime publish number-baseball \
  --server local \
  --module-path spacetimedb \
  --delete-data=always \
  --yes
spacetime generate \
  --lang typescript \
  --out-dir src/module_bindings \
  --module-path spacetimedb
npm install
npm run dev
```

기본 클라이언트 주소는 `ws://localhost:3000`, 데이터베이스 이름은
`number-baseball`입니다. 필요한 경우 다음 환경 변수로 변경할 수 있습니다.

```bash
VITE_SPACETIMEDB_HOST=wss://your-host.example
VITE_SPACETIMEDB_DB_NAME=your-database
```

## 검증

```bash
npm run build
npm test
npx tsc --noEmit -p spacetimedb/tsconfig.json
```

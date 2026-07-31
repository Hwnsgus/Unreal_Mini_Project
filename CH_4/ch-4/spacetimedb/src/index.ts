import { schema, SenderError, t, table } from "spacetimedb/server";

const MAX_ATTEMPTS = 3;
const GAME_ID = 1;

const player = table(
  { name: "player", public: true },
  {
    identity: t.identity().primaryKey(),
    name: t.string(),
    attempts_left: t.u32(),
    online: t.bool(),
  },
);

const game_state = table(
  { name: "game_state", public: true },
  {
    id: t.u32().primaryKey(),
    round: t.u32(),
    status: t.string(),
    winner_name: t.string().optional(),
  },
);

// This table is intentionally private. Clients must never be able to subscribe
// to the answer.
const game_secret = table(
  { name: "game_secret" },
  {
    id: t.u32().primaryKey(),
    answer: t.string(),
  },
);

const guess_result = table(
  { name: "guess_result", public: true },
  {
    id: t.u64().primaryKey().autoInc(),
    round: t.u32().index("btree"),
    player_identity: t.identity(),
    player_name: t.string(),
    guess: t.string(),
    strikes: t.u32(),
    balls: t.u32(),
    is_out: t.bool(),
    attempts_left: t.u32(),
    created_at: t.timestamp(),
  },
);

const spacetimedb = schema({
  player,
  game_state,
  game_secret,
  guess_result,
});

export default spacetimedb;

function generateAnswer(ctx: {
  random: { integerInRange(min: number, max: number): number };
}) {
  const digits: number[] = [];

  while (digits.length < 3) {
    const digit = ctx.random.integerInRange(1, 9);
    if (!digits.includes(digit)) digits.push(digit);
  }

  return digits.join("");
}

function validateName(value: string) {
  const name = value.trim();
  if (!name) throw new SenderError("이름을 입력해주세요.");
  if (name.length > 20) {
    throw new SenderError("이름은 20자 이하로 입력해주세요.");
  }
  return name;
}

function validateGuess(value: string) {
  const guess = value.trim();

  if (!/^[1-9]{3}$/.test(guess)) {
    throw new SenderError("1부터 9까지의 숫자 3개를 입력해주세요.");
  }

  if (new Set(guess).size !== 3) {
    throw new SenderError("중복되지 않은 숫자 3개를 입력해주세요.");
  }

  return guess;
}

function scoreGuess(answer: string, guess: string) {
  let strikes = 0;
  let balls = 0;

  for (let index = 0; index < 3; index++) {
    if (guess[index] === answer[index]) {
      strikes++;
    } else if (answer.includes(guess[index])) {
      balls++;
    }
  }

  return {
    strikes,
    balls,
    isOut: strikes === 0 && balls === 0,
  };
}

export const set_name = spacetimedb.reducer(
  { name: t.string() },
  (ctx, { name }) => {
    const currentPlayer = ctx.db.player.identity.find(ctx.sender);
    if (!currentPlayer) {
      throw new SenderError("접속 중인 플레이어를 찾을 수 없습니다.");
    }

    ctx.db.player.identity.update({
      ...currentPlayer,
      name: validateName(name),
    });
  },
);

export const submit_guess = spacetimedb.reducer(
  { guess: t.string() },
  (ctx, { guess }) => {
    // Validate first. Throwing SenderError aborts the transaction, so an
    // invalid guess never consumes an attempt.
    const validGuess = validateGuess(guess);
    const currentPlayer = ctx.db.player.identity.find(ctx.sender);
    const game = ctx.db.game_state.id.find(GAME_ID);
    const secret = ctx.db.game_secret.id.find(GAME_ID);

    if (!currentPlayer) {
      throw new SenderError("접속 중인 플레이어를 찾을 수 없습니다.");
    }
    if (!game || !secret) {
      throw new SenderError("게임이 아직 준비되지 않았습니다.");
    }
    if (game.status !== "playing") {
      throw new SenderError("현재 라운드가 종료되었습니다.");
    }
    if (currentPlayer.attempts_left === 0) {
      throw new SenderError("이번 라운드의 기회를 모두 사용했습니다.");
    }

    const result = scoreGuess(secret.answer, validGuess);
    const attemptsLeft = currentPlayer.attempts_left - 1;

    ctx.db.player.identity.update({
      ...currentPlayer,
      attempts_left: attemptsLeft,
    });

    ctx.db.guess_result.insert({
      id: 0n,
      round: game.round,
      player_identity: ctx.sender,
      player_name: currentPlayer.name,
      guess: validGuess,
      strikes: result.strikes,
      balls: result.balls,
      is_out: result.isOut,
      attempts_left: attemptsLeft,
      created_at: ctx.timestamp,
    });

    if (result.strikes === 3) {
      ctx.db.game_state.id.update({
        ...game,
        status: "won",
        winner_name: currentPlayer.name,
      });
      return;
    }

    const onlinePlayers = [...ctx.db.player.iter()].filter((row) => row.online);
    const everybodyFinished =
      onlinePlayers.length > 0 &&
      onlinePlayers.every((row) =>
        row.identity.isEqual(ctx.sender)
          ? attemptsLeft === 0
          : row.attempts_left === 0,
      );

    if (everybodyFinished) {
      ctx.db.game_state.id.update({
        ...game,
        status: "draw",
        winner_name: undefined,
      });
    }
  },
);

export const start_new_round = spacetimedb.reducer((ctx) => {
  const game = ctx.db.game_state.id.find(GAME_ID);
  const secret = ctx.db.game_secret.id.find(GAME_ID);

  if (!game || !secret) {
    throw new SenderError("게임이 아직 준비되지 않았습니다.");
  }
  if (game.status === "playing") {
    throw new SenderError("진행 중인 라운드는 다시 시작할 수 없습니다.");
  }

  ctx.db.game_secret.id.update({
    ...secret,
    answer: generateAnswer(ctx),
  });
  ctx.db.game_state.id.update({
    ...game,
    round: game.round + 1,
    status: "playing",
    winner_name: undefined,
  });

  for (const row of ctx.db.player.iter()) {
    ctx.db.player.identity.update({
      ...row,
      attempts_left: MAX_ATTEMPTS,
    });
  }
});

export const init = spacetimedb.init((ctx) => {
  ctx.db.game_state.insert({
    id: GAME_ID,
    round: 1,
    status: "playing",
    winner_name: undefined,
  });
  ctx.db.game_secret.insert({
    id: GAME_ID,
    answer: generateAnswer(ctx),
  });
});

export const onConnect = spacetimedb.clientConnected((ctx) => {
  const currentPlayer = ctx.db.player.identity.find(ctx.sender);

  if (currentPlayer) {
    ctx.db.player.identity.update({
      ...currentPlayer,
      online: true,
    });
    return;
  }

  ctx.db.player.insert({
    identity: ctx.sender,
    name: `Player-${ctx.sender.toHexString().slice(0, 6)}`,
    attempts_left: MAX_ATTEMPTS,
    online: true,
  });
});

export const onDisconnect = spacetimedb.clientDisconnected((ctx) => {
  const currentPlayer = ctx.db.player.identity.find(ctx.sender);
  if (!currentPlayer) return;

  ctx.db.player.identity.update({
    ...currentPlayer,
    online: false,
  });
});

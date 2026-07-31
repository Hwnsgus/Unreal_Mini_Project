import { FormEvent, useMemo, useState } from "react";
import { useReducer, useSpacetimeDB, useTable } from "spacetimedb/react";
import "./App.css";
import { reducers, tables } from "./module_bindings";

function getErrorMessage(reason: unknown) {
  if (!(reason instanceof Error)) return "요청을 처리하지 못했습니다.";

  const message = reason.message;
  const senderError = message.match(/SenderError:\s*(.+?)(?:\n|$)/);
  return senderError?.[1] ?? message;
}

function resultLabel(result: {
  strikes: number;
  balls: number;
  isOut: boolean;
}) {
  return result.isOut ? "OUT" : `${result.strikes}S ${result.balls}B`;
}

function App() {
  const { identity, isActive } = useSpacetimeDB();
  const [gameStates] = useTable(tables.gameState);
  const [players] = useTable(tables.player);
  const [guessResults] = useTable(tables.guessResult);
  const setName = useReducer(reducers.setName);
  const submitGuessReducer = useReducer(reducers.submitGuess);
  const startNewRound = useReducer(reducers.startNewRound);

  const [guess, setGuess] = useState("");
  const [nameDraft, setNameDraft] = useState("");
  const [editingName, setEditingName] = useState(false);
  const [error, setError] = useState("");
  const [submitting, setSubmitting] = useState(false);

  const game = gameStates[0];
  const currentPlayer = identity
    ? players.find((row) => row.identity.isEqual(identity))
    : undefined;
  const onlinePlayers = players.filter((row) => row.online);

  const roundResults = useMemo(() => {
    if (!game) return [];

    return guessResults
      .filter((row) => row.round === game.round)
      .sort((a, b) => {
        const aTime = a.createdAt.microsSinceUnixEpoch;
        const bTime = b.createdAt.microsSinceUnixEpoch;
        return aTime > bTime ? -1 : aTime < bTime ? 1 : 0;
      });
  }, [game, guessResults]);

  const canGuess =
    game?.status === "playing" &&
    currentPlayer !== undefined &&
    currentPlayer.attemptsLeft > 0 &&
    !submitting;

  const submitGuess = async (event: FormEvent) => {
    event.preventDefault();
    if (!guess.trim() || !canGuess) return;

    setSubmitting(true);
    setError("");
    try {
      await submitGuessReducer({ guess });
      setGuess("");
    } catch (reason) {
      setError(getErrorMessage(reason));
    } finally {
      setSubmitting(false);
    }
  };

  const submitName = async (event: FormEvent) => {
    event.preventDefault();
    if (!nameDraft.trim()) return;

    setError("");
    try {
      await setName({ name: nameDraft });
      setEditingName(false);
    } catch (reason) {
      setError(getErrorMessage(reason));
    }
  };

  const restart = async () => {
    setError("");
    try {
      await startNewRound();
      setGuess("");
    } catch (reason) {
      setError(getErrorMessage(reason));
    }
  };

  if (!isActive || !identity) {
    return (
      <main className="connection-screen">
        <div className="baseball-mark" aria-hidden="true">
          ⚾
        </div>
        <h1>숫자 야구 경기장에 입장 중</h1>
        <p>SpaceTimeDB 실시간 서버에 연결하고 있습니다.</p>
        <span className="loading-dots" aria-label="서버 연결 중">
          <i />
          <i />
          <i />
        </span>
      </main>
    );
  }

  return (
    <main className="game-page">
      <div className="stadium-glow stadium-glow-left" />
      <div className="stadium-glow stadium-glow-right" />

      <header className="topbar">
        <a className="brand" href="/" aria-label="홈런 숫자 야구 홈">
          <span>⚾</span>
          <strong>HOME RUN!</strong>
          <small>NUMBER BASEBALL</small>
        </a>

        <div className="round-chip">
          <span>ROUND</span>
          <strong>{game?.round ?? "-"}</strong>
        </div>

        {editingName ? (
          <form className="name-editor" onSubmit={submitName}>
            <input
              autoFocus
              aria-label="플레이어 이름"
              maxLength={20}
              value={nameDraft}
              onChange={(event) => setNameDraft(event.target.value)}
            />
            <button type="submit">저장</button>
          </form>
        ) : (
          <button
            className="player-profile"
            onClick={() => {
              setNameDraft(currentPlayer?.name ?? "");
              setEditingName(true);
            }}
          >
            <span className="online-dot" />
            <span>
              <small>내 선수</small>
              <strong>{currentPlayer?.name ?? "연결 중..."}</strong>
            </span>
            <b>✎</b>
          </button>
        )}
      </header>

      <section className="hero">
        <div>
          <p className="eyebrow">REALTIME MULTIPLAYER</p>
          <h1>
            세 개의 숫자로
            <br />
            <em>홈런</em>을 노려라!
          </h1>
          <p className="hero-copy">
            서버가 숨긴 서로 다른 숫자 세 개를 맞혀보세요.
            <br />
            숫자와 자리까지 맞으면 스트라이크입니다.
          </p>
        </div>

        <div className="scoreboard">
          <div className="scoreboard-lights">
            <i />
            <i />
            <i />
          </div>
          <span>MY CHANCES</span>
          <strong>
            {currentPlayer?.attemptsLeft ?? 0}
            <small>/ 3</small>
          </strong>
          <div className="attempt-balls" aria-label="남은 기회">
            {[0, 1, 2].map((index) => (
              <i
                className={
                  index < (currentPlayer?.attemptsLeft ?? 0) ? "active" : ""
                }
                key={index}
              >
                ⚾
              </i>
            ))}
          </div>
        </div>
      </section>

      {game?.status !== "playing" && (
        <section className={`game-banner ${game?.status ?? ""}`}>
          <div>
            <span>{game?.status === "won" ? "GAME OVER" : "NO WINNER"}</span>
            <h2>
              {game?.status === "won"
                ? `${game.winnerName} 선수의 홈런!`
                : "이번 경기는 무승부입니다."}
            </h2>
          </div>
          <button onClick={restart}>다음 라운드 시작</button>
        </section>
      )}

      <section className="game-grid">
        <article className="play-card">
          <div className="card-heading">
            <div>
              <span className="card-number">01</span>
              <p>YOUR GUESS</p>
              <h2>타석에 들어서세요</h2>
            </div>
            <span className="live-badge">
              <i /> LIVE
            </span>
          </div>

          <form className="guess-form" onSubmit={submitGuess}>
            <label htmlFor="guess">서로 다른 1~9 숫자 세 개</label>
            <div className="guess-input-wrap">
              <input
                id="guess"
                aria-label="추측 숫자"
                inputMode="numeric"
                maxLength={3}
                placeholder="123"
                value={guess}
                disabled={!canGuess}
                onChange={(event) =>
                  setGuess(event.target.value.replace(/\D/g, "").slice(0, 3))
                }
              />
              <button disabled={!canGuess || guess.length !== 3} type="submit">
                {submitting ? "판정 중" : "SWING"}
                <span>➜</span>
              </button>
            </div>
          </form>

          {error && (
            <button className="error-message" onClick={() => setError("")}>
              <span>!</span>
              {error}
              <b>×</b>
            </button>
          )}

          <div className="rules">
            <div>
              <strong>S</strong>
              <span>
                <b>STRIKE</b>
                숫자와 자리 일치
              </span>
            </div>
            <div>
              <strong>B</strong>
              <span>
                <b>BALL</b>
                숫자만 일치
              </span>
            </div>
            <div>
              <strong>O</strong>
              <span>
                <b>OUT</b>
                일치 숫자 없음
              </span>
            </div>
          </div>
        </article>

        <article className="history-card">
          <div className="card-heading">
            <div>
              <span className="card-number">02</span>
              <p>PLAY BY PLAY</p>
              <h2>실시간 판정 기록</h2>
            </div>
            <span className="record-count">{roundResults.length}</span>
          </div>

          <div className="history-list">
            {roundResults.length === 0 ? (
              <div className="empty-state">
                <span>◇</span>
                <strong>아직 투구 기록이 없습니다</strong>
                <p>첫 번째 숫자를 입력해 경기를 시작하세요.</p>
              </div>
            ) : (
              roundResults.map((result, index) => (
                <div className="history-row" key={result.id.toString()}>
                  <span className="history-index">
                    {String(roundResults.length - index).padStart(2, "0")}
                  </span>
                  <div className="history-player">
                    <strong>{result.playerName}</strong>
                    <small>남은 기회 {result.attemptsLeft}/3</small>
                  </div>
                  <code>{result.guess}</code>
                  <b
                    className={
                      result.strikes === 3
                        ? "home-run"
                        : result.isOut
                          ? "out"
                          : ""
                    }
                  >
                    {resultLabel(result)}
                  </b>
                </div>
              ))
            )}
          </div>
        </article>

        <aside className="players-card">
          <div className="players-title">
            <div>
              <p>CLUBHOUSE</p>
              <h2>접속 선수</h2>
            </div>
            <span>{onlinePlayers.length}</span>
          </div>

          <div className="player-list">
            {onlinePlayers.map((player, index) => {
              const isMe = player.identity.isEqual(identity);
              return (
                <div
                  className={`player-row ${isMe ? "me" : ""}`}
                  key={player.identity.toHexString()}
                >
                  <span className="uniform-number">
                    {String(index + 1).padStart(2, "0")}
                  </span>
                  <div>
                    <strong>{player.name}</strong>
                    <small>{isMe ? "나" : "온라인"}</small>
                  </div>
                  <span className="mini-attempts">
                    {player.attemptsLeft} <small>OUTS</small>
                  </span>
                </div>
              );
            })}
          </div>

          <div className="server-note">
            <span>✓</span>
            <p>
              <strong>SERVER AUTHORITATIVE</strong>
              정답과 판정은 SpaceTimeDB 서버에서만 처리됩니다.
            </p>
          </div>
        </aside>
      </section>
    </main>
  );
}

export default App;

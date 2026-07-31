import { render, screen } from "@testing-library/react";
import { describe, expect, it } from "vitest";
import { SpacetimeDBProvider } from "spacetimedb/react";
import App from "./App";
import { DbConnection } from "./module_bindings";

describe("숫자 야구 앱", () => {
  it("실시간 세션이 열리는 동안 연결 화면을 표시한다", () => {
    const connectionBuilder = DbConnection.builder()
      .withUri("ws://localhost:3000")
      .withDatabaseName("number-baseball");

    render(
      <SpacetimeDBProvider connectionBuilder={connectionBuilder}>
        <App />
      </SpacetimeDBProvider>,
    );

    expect(
      screen.getByRole("heading", { name: "숫자 야구 경기장에 입장 중" }),
    ).toBeInTheDocument();
    expect(screen.getByLabelText("서버 연결 중")).toBeInTheDocument();
  });
});

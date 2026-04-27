import type { CompileResult } from "./types";

const BASE = "http://localhost:8080";

export async function compile(source: string): Promise<CompileResult> {
  const res = await fetch(`${BASE}/compile`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ source }),
  });
  return res.json();
}

export async function checkHealth(): Promise<{ ok: boolean }> {
  try {
    const res = await fetch(`${BASE}/health`);
    return res.json();
  } catch {
    return { ok: false };
  }
}

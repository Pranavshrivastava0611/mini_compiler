"use client";

import { useState, useEffect, useCallback } from "react";
import { compile, checkHealth } from "@/lib/api";
import type { CompileResult, AstNode as AstNodeType } from "@/lib/types";
import {
  SkipForward,
  X,
  Server,
  AlertCircle,
  Code2,
  TreeDeciduous,
  Cpu,
  Terminal,
  Layers,
  Zap,
  RotateCcw,
} from "lucide-react";

// ── Example Programs ──────────────────────────────────────────
const EXAMPLES = [
  {
    name: "Arithmetic",
    code: `# Basic Arithmetic
let a = 5
let b = 10
print a + b * 2
print 2 ^ 8
let hyp = (a * a + b * b) ^ 0.5
print hyp`,
  },
  {
    name: "Division",
    code: `# Division & Grouping
let x = 100
let y = 3
print x / y
print x - y * y
print (x + y) * 2`,
  },
  {
    name: "Powers",
    code: `# Powers of Two
let base = 2
print base ^ 1
print base ^ 2
print base ^ 3
print base ^ 4
print base ^ 8
print base ^ 10
print base ^ 16`,
  },
  {
    name: "Negative",
    code: `# Unary Negation
let val = 42
print -val
print -(-val)
print val + -10`,
  },
];

// ── Main Page Component ───────────────────────────────────────
export default function HomePage() {
  const [source, setSource] = useState(EXAMPLES[0].code);
  const [activeExample, setActiveExample] = useState(0);
  const [result, setResult] = useState<CompileResult | null>(null);
  const [serverOk, setServerOk] = useState(false);

  // Step-mode state
  const [stepIndex, setStepIndex] = useState(-1);
  const [isStepMode, setIsStepMode] = useState(false);

  // Compile with debounce
  const handleCompile = useCallback(
    async (code: string) => {
      try {
        const res = await compile(code);
        setResult(res);
        if (!isStepMode) setStepIndex(-1);
      } catch (e) {
        console.error(e);
      }
    },
    [isStepMode]
  );

  useEffect(() => {
    const timer = setTimeout(() => handleCompile(source), 350);
    return () => clearTimeout(timer);
  }, [source, handleCompile]);

  // Health check
  useEffect(() => {
    const check = async () => {
      const res = await checkHealth();
      setServerOk(res.ok);
    };
    check();
    const interval = setInterval(check, 5000);
    return () => clearInterval(interval);
  }, []);

  // Step mode controls
  const startStepMode = () => {
    setIsStepMode(true);
    setStepIndex(0);
  };
  const exitStepMode = () => {
    setIsStepMode(false);
    setStepIndex(-1);
  };
  const nextStep = () => {
    if (result && stepIndex < result.executionTrace.length - 1)
      setStepIndex((s) => s + 1);
  };
  const prevStep = () => {
    if (stepIndex > 0) setStepIndex((s) => s - 1);
  };
  const runAll = () => {
    if (result) setStepIndex(result.executionTrace.length - 1);
  };
  const resetSteps = () => setStepIndex(0);

  const currentTrace =
    result && stepIndex >= 0 ? result.executionTrace[stepIndex] : null;
  const currentIp = currentTrace ? currentTrace.ip : -1;

  const lineCount = source.split("\n").length;

  const selectExample = (idx: number) => {
    setActiveExample(idx);
    setSource(EXAMPLES[idx].code);
    exitStepMode();
  };

  return (
    <div className="app-shell">
      {/* ═══ HEADER ═══ */}
      <header className="app-header">
        <div className="logo-mark">⚡</div>
        <span className="header-title">vm_lang</span>
        <span className="header-subtitle">
          bytecode compiler + stack VM
        </span>
        <div className="header-actions">
          <div
            className={`status-badge ${serverOk ? "online" : "offline"}`}
          >
            <div className="status-dot" />
            <Server size={12} />
            {serverOk ? "Online" : "Offline"}
          </div>
        </div>
      </header>

      {/* ═══ MAIN GRID ═══ */}
      <main className={`main-layout ${isStepMode ? "has-trace" : ""}`}>
        {/* ── Panel 1: Source Editor ── */}
        <div className="panel editor-panel">
          <div className="panel-header">
            <div className="label">
              <Code2 size={14} className="icon" /> Source Editor
            </div>
            {result && (
              <span
                className={`badge ${result.error ? "badge-error" : "badge-success"}`}
              >
                {result.error ? "✗ Error" : "✓ Compiled"}
              </span>
            )}
          </div>

          {result?.error && (
            <div className="error-banner">
              <AlertCircle size={14} />
              <span>{result.error}</span>
            </div>
          )}

          <div className="editor-wrapper">
            <div className="line-numbers">
              {Array.from({ length: lineCount }, (_, i) => (
                <div key={i} className="line-number">
                  {i + 1}
                </div>
              ))}
            </div>
            <textarea
              id="source-editor"
              className="source-editor"
              value={source}
              onChange={(e) => setSource(e.target.value)}
              spellCheck={false}
              autoCapitalize="off"
              autoCorrect="off"
            />
          </div>

          <div className="editor-toolbar">
            <button
              className="btn btn-primary"
              onClick={startStepMode}
              disabled={!result || !!result.error || isStepMode}
              id="step-mode-btn"
            >
              <Zap size={13} /> Debug
            </button>

            <div style={{ flex: 1 }} />

            {EXAMPLES.map((ex, i) => (
              <button
                key={i}
                className={`btn example-btn ${activeExample === i ? "active" : ""}`}
                onClick={() => selectExample(i)}
                id={`example-btn-${i}`}
              >
                {ex.name}
              </button>
            ))}
          </div>
        </div>

        {/* ── Panel 2: Tokens + AST ── */}
        <div className="panel" style={{ borderRight: "none" }}>
          <div className="split-h">
            {/* Tokens */}
            <div>
              <div className="panel-header">
                <div className="label">
                  <Layers size={14} className="icon" /> Tokens
                </div>
                {result?.tokens && (
                  <span className="badge badge-info">
                    {
                      result.tokens.filter(
                        (t) => t.kind !== "Newline" && t.kind !== "Eof"
                      ).length
                    }
                  </span>
                )}
              </div>
              <div className="scroll-area">
                <div className="token-grid">
                  {result?.tokens?.map((t, i) => (
                    <div
                      key={i}
                      className={`token-chip ${t.kind}`}
                      title={`${t.kind} · line ${t.line}${t.value !== undefined ? ` · value: ${t.value}` : ""}`}
                    >
                      {t.kind === "Newline"
                        ? "↵"
                        : t.kind === "Eof"
                          ? "EOF"
                          : t.text}
                    </div>
                  ))}
                </div>
              </div>
            </div>

            {/* AST */}
            <div>
              <div className="panel-header">
                <div className="label">
                  <TreeDeciduous size={14} className="icon" /> AST
                </div>
              </div>
              <div className="scroll-area ast-tree">
                {result?.ast ? (
                  <AstNodeView node={result.ast} depth={0} />
                ) : (
                  <div className="empty-state">
                    <TreeDeciduous size={24} className="icon" />
                    <span>Write code to see the AST</span>
                  </div>
                )}
              </div>
            </div>
          </div>
        </div>

        {/* ── Panel 3: Bytecode ── */}
        <div className="panel">
          <div className="panel-header">
            <div className="label">
              <Cpu size={14} className="icon" /> Bytecode
            </div>
            {isStepMode && (
              <div className="step-controls">
                <button
                  className="btn btn-sm"
                  onClick={resetSteps}
                  id="reset-btn"
                  title="Reset"
                >
                  <RotateCcw size={11} />
                </button>
                <button
                  className="btn btn-sm"
                  onClick={prevStep}
                  disabled={stepIndex <= 0}
                  id="prev-step-btn"
                >
                  ← Prev
                </button>
                <button
                  className="btn btn-sm btn-primary"
                  onClick={nextStep}
                  disabled={
                    !result ||
                    stepIndex >= result.executionTrace.length - 1
                  }
                  id="next-step-btn"
                >
                  Next →
                </button>
                <button
                  className="btn btn-sm"
                  onClick={runAll}
                  id="run-all-btn"
                >
                  <SkipForward size={11} /> End
                </button>
                <span className="step-counter">
                  {stepIndex + 1}/{result?.executionTrace.length || 0}
                </span>
                <button
                  className="btn btn-sm btn-danger"
                  onClick={exitStepMode}
                  id="exit-step-btn"
                >
                  <X size={11} />
                </button>
              </div>
            )}
          </div>
          <div className="bytecode-wrap">
            <table className="bytecode-table">
              <thead>
                <tr>
                  <th>IP</th>
                  <th>Opcode</th>
                  <th>Arg</th>
                  <th>Comment</th>
                </tr>
              </thead>
              <tbody>
                {result?.bytecode?.instructions?.map((ins) => (
                  <tr
                    key={ins.ip}
                    className={`bytecode-row ${currentIp === ins.ip ? "active" : ""}`}
                  >
                    <td className="ip-cell">
                      {String(ins.ip).padStart(3, "0")}
                    </td>
                    <td className="op-cell">{ins.op}</td>
                    <td className="arg-cell">
                      {ins.arg === -1 ? "—" : ins.arg}
                    </td>
                    <td className="comment-cell">{ins.comment}</td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
          {result?.bytecode?.constants && result.bytecode.constants.length > 0 && (
            <div className="constant-pool">
              <div className="pool-label">Constant Pool</div>
              <div className="pool-items">
                {result.bytecode.constants.map((c, i) => (
                  <div key={i} className="pool-item">
                    <span className="idx">[{i}]</span> ={" "}
                    <span className="val">{c}</span>
                  </div>
                ))}
              </div>
            </div>
          )}
        </div>

        {/* ── Panel 4: VM State + Output ── */}
        <div className="panel" style={{ borderRight: "none" }}>
          <div className="vm-state-grid">
            {/* Stack + Vars */}
            <div className="vm-section">
              <div className="vm-section-header">
                Stack{" "}
                {currentTrace && `(${currentTrace.stackAfter.length})`}
              </div>
              <div className="vm-section-body">
                {currentTrace && currentTrace.stackAfter.length > 0 ? (
                  <div className="stack-visual">
                    {currentTrace.stackAfter.map((val, i) => (
                      <div key={i} className="stack-slot">
                        <span className="slot-idx">{i}</span>
                        <span>{val}</span>
                      </div>
                    ))}
                  </div>
                ) : (
                  <div className="empty-state">
                    <Layers size={20} className="icon" />
                    <span>
                      {isStepMode
                        ? "Stack empty"
                        : "Enter debug mode to inspect"}
                    </span>
                  </div>
                )}
              </div>

              <div className="vm-section-header">Variables</div>
              <div className="vm-section-body">
                {currentTrace &&
                Object.keys(currentTrace.vars).length > 0 ? (
                  <div className="var-list">
                    {Object.entries(currentTrace.vars).map(
                      ([name, val]) => (
                        <div key={name} className="var-entry">
                          <span className="var-name">{name}</span>
                          <span className="var-value">{val}</span>
                        </div>
                      )
                    )}
                  </div>
                ) : (
                  <div className="empty-state">
                    <span>
                      {isStepMode ? "No variables yet" : ""}
                    </span>
                  </div>
                )}
              </div>
            </div>

            {/* Output */}
            <div className="vm-section">
              <div className="vm-section-header">
                <Terminal size={12} />
                Output
              </div>
              <div className="vm-section-body output-terminal">
                {(
                  isStepMode && currentTrace
                    ? currentTrace.output
                    : result?.output || []
                ).length > 0 ? (
                  (isStepMode && currentTrace
                    ? currentTrace.output
                    : result?.output || []
                  ).map((line, i) => (
                    <div
                      key={`${i}-${line}`}
                      className="output-line fade-in-up"
                    >
                      <span className="output-prefix">❯</span>
                      <span className="output-value">{line}</span>
                    </div>
                  ))
                ) : (
                  <div className="empty-state" style={{ color: "#2a2a2a" }}>
                    <Terminal size={20} className="icon" />
                    <span>No output</span>
                  </div>
                )}
              </div>
            </div>
          </div>
        </div>
      </main>

      {/* ═══ TRACE DRAWER ═══ */}
      {isStepMode && result && (
        <div className="trace-drawer">
          <div className="panel-header">
            <div className="label">
              <Zap size={14} className="icon" /> Execution Trace
            </div>
            <span className="step-counter">
              Step {stepIndex + 1} of {result.executionTrace.length}
            </span>
          </div>
          <div className="trace-list">
            {result.executionTrace
              .slice(0, stepIndex + 1)
              .map((step, i) => (
                <div
                  key={i}
                  className={`trace-entry ${i === stepIndex ? "active" : ""}`}
                  onClick={() => setStepIndex(i)}
                >
                  <span className="trace-ip">ip={step.ip}</span>
                  <span className="trace-op">{step.op}</span>
                  <span className="trace-stack">
                    [{step.stackBefore.join(", ")}]
                    <span className="arrow">→</span>[
                    {step.stackAfter.join(", ")}]
                  </span>
                  <span className="trace-vars">
                    {Object.entries(step.vars)
                      .map(([k, v]) => `${k}=${v}`)
                      .join(" ")}
                  </span>
                </div>
              ))}
          </div>
        </div>
      )}
    </div>
  );
}

// ── AST Node Recursive Component ──────────────────────────────
function AstNodeView({
  node,
  depth,
}: {
  node: AstNodeType;
  depth: number;
}) {
  const [collapsed, setCollapsed] = useState(false);

  const hasChildren = !!(
    node.stmts ||
    node.left ||
    node.right ||
    node.expr ||
    node.operand
  );

  return (
    <div
      className="ast-node"
      style={
        depth === 0
          ? { marginLeft: 0, borderLeft: "none" }
          : undefined
      }
    >
      <div
        className="ast-label"
        onClick={() => hasChildren && setCollapsed(!collapsed)}
      >
        {hasChildren && (
          <span
            className={`ast-toggle ${collapsed ? "collapsed" : ""}`}
          >
            ▼
          </span>
        )}
        {!hasChildren && (
          <span style={{ width: 14, display: "inline-block" }} />
        )}
        <span className={`ast-type ${node.type}`}>{node.type}</span>
        {node.name && (
          <span
            className="ast-value"
            style={{ color: "var(--accent-amber)" }}
          >
            {node.name}
          </span>
        )}
        {node.value !== undefined && (
          <span
            className="ast-value"
            style={{ color: "var(--accent-emerald)" }}
          >
            {node.value}
          </span>
        )}
        {node.op && (
          <span
            className="ast-value"
            style={{ color: "var(--accent-violet)" }}
          >
            &quot;{node.op}&quot;
          </span>
        )}
      </div>
      {!collapsed && (
        <>
          {node.stmts?.map((s, i) => (
            <AstNodeView key={i} node={s} depth={depth + 1} />
          ))}
          {node.expr && (
            <AstNodeView node={node.expr} depth={depth + 1} />
          )}
          {node.operand && (
            <AstNodeView node={node.operand} depth={depth + 1} />
          )}
          {node.left && (
            <AstNodeView node={node.left} depth={depth + 1} />
          )}
          {node.right && (
            <AstNodeView node={node.right} depth={depth + 1} />
          )}
        </>
      )}
    </div>
  );
}

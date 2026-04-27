export interface Token {
  kind: string;
  text: string;
  value?: number;
  line: number;
}

export interface AstNode {
  type: string;
  value?: number;
  name?: string;
  op?: string;
  left?: AstNode;
  right?: AstNode;
  operand?: AstNode;
  expr?: AstNode;
  stmts?: AstNode[];
}

export interface Instruction {
  ip: number;
  op: string;
  arg: number;
  comment: string;
}

export interface TraceStep {
  ip: number;
  op: string;
  arg: number;
  stackBefore: number[];
  stackAfter: number[];
  vars: Record<string, number>;
  output: string[];
}

export interface CompileResult {
  tokens: Token[];
  ast: AstNode;
  bytecode: {
    instructions: Instruction[];
    constants: number[];
    varNames: string[];
  };
  executionTrace: TraceStep[];
  output: string[];
  error: string | null;
}

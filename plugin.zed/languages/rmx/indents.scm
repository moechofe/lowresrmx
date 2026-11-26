(if_statement
  "END IF" @end) @indent

(for_statement
  "NEXT" @end) @indent

(do_statement
  "LOOP" @end) @indent

(while_statement
  "WEND" @end) @indent

(repeat_statement
  "UNTIL" @end) @indent

(sub_definition
  "END SUB" @end) @indent

[
  (else_if_clause)
  (else_clause)
] @outdent

[
  (else_if_clause)
  (else_clause)
] @indent

(_
  "("
  ")" @end) @indent

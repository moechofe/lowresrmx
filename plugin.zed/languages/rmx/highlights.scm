(identifier) @variable

(string_identifier) @variable

(comment) @comment

((comment) @comment.doc
  (#match? @comment.doc "^'''"))

(string) @string

(number) @number

(label
  name: (identifier) @label)

(label_ref) @label

(sub_definition
  name: (identifier) @function)

(call_statement
  name: (identifier) @function)

(on_statement
  procedure: (identifier) @function)

(parameter
  name: (_) @variable.parameter)

(command
  name: _ @function.builtin)

(builtin_call
  name: _ @function.builtin)

(command
  keyword: _ @keyword)

(modifier) @keyword

[
  "CALL"
  "DO"
  "ELSE"
  "END"
  "END IF"
  "END SUB"
  "EXIT"
  "FOR"
  "GOSUB"
  "GOTO"
  "IF"
  "LOOP"
  "NEXT"
  "OFF"
  "ON"
  "REPEAT"
  "RESTORE"
  "RETURN"
  "STEP"
  "SUB"
  "THEN"
  "TO"
  "UNTIL"
  "WEND"
  "WHILE"
] @keyword

[
  "AND"
  "MOD"
  "NOT"
  "OR"
  "XOR"
] @keyword.operator

[
  "="
  "<>"
  "<"
  ">"
  "<="
  ">="
  "+"
  "-"
  "*"
  "/"
  "\\"
  "^"
] @operator

[
  "("
  ")"
] @punctuation.bracket

[
  ","
  ";"
  ":"
] @punctuation.delimiter

(rom_entry
  "#" @preproc
  ":" @preproc)

(rom_slot) @preproc

(rom_comment) @string.special

(rom_data) @comment

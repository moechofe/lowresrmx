("(" @open
  ")" @close)

((if_statement
  "IF" @open
  "END IF" @close)
  (#set! rainbow.exclude))

((for_statement
  "FOR" @open
  "NEXT" @close)
  (#set! rainbow.exclude))

((do_statement
  "DO" @open
  "LOOP" @close)
  (#set! rainbow.exclude))

((while_statement
  "WHILE" @open
  "WEND" @close)
  (#set! rainbow.exclude))

((repeat_statement
  "REPEAT" @open
  "UNTIL" @close)
  (#set! rainbow.exclude))

((sub_definition
  "SUB" @open
  "END SUB" @close)
  (#set! rainbow.exclude))

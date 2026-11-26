(sub_definition
  "SUB" @context
  name: (identifier) @name) @item

(label
  name: (identifier) @name) @item

((comment) @item @name
  (#match? @name "^'''"))

(rom_entry
  "#" @context
  slot: (rom_slot) @name
  comment: (rom_comment)? @context) @item

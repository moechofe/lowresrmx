/**
 * @file Tree-sitter grammar for the LowResRMX / Retro Game Creator BASIC dialect.
 *
 * Mirrors `backend.core/interpreter/tokenizer.c`: keywords are case-insensitive, `:` separates
 * instructions like a newline does, `name:` glued to its colon declares a label, `'` starts a
 * comment, and the first `#` outside a string or comment ends the program and opens the ROM
 * entries (`#slot:comment` followed by lines of hexadecimal).
 *
 * Every keyword of `TokenStrings[]` in `backend.core/interpreter/token.c` must sit in exactly one
 * of the lists below: `tree-sitter generate` fails otherwise, so a keyword added to the engine
 * cannot silently go unhighlighted.
 */

const fs = require('fs');
const path = require('path');

const STRUCTURE = [
  'CALL', 'DO', 'ELSE', 'END', 'EXIT', 'FOR', 'GOSUB', 'GOTO', 'IF', 'LOOP', 'NEXT', 'OFF', 'ON',
  'REPEAT', 'RESTORE', 'RETURN', 'STEP', 'SUB', 'THEN', 'TO', 'UNTIL', 'WEND', 'WHILE',
];

const OPERATORS = ['AND', 'MOD', 'NOT', 'OR', 'XOR'];

// Instructions of the language itself, highlighted as keywords.
const STATEMENTS = ['ADD', 'ASSERT', 'DATA', 'DEC', 'DIM', 'GLOBAL', 'INC', 'READ', 'SKIP', 'SWAP'];

// API commands, highlighted as built-in functions.
const COMMANDS = [
  'ATTR', 'BG', 'CELL', 'CLS', 'CLW', 'COMPAT', 'COPY', 'DMA', 'EMITTER', 'ENVELOPE', 'FILES',
  'FILL', 'FLIP', 'FONT', 'HAPTIC', 'INPUT', 'KEYBOARD', 'LFO', 'LFO.A', 'LOAD', 'LOCATE', 'MCELL',
  'MESSAGE', 'MUSIC', 'NUMBER', 'PAL', 'PALETTE', 'PARTICLE', 'PAUSE', 'PLAY', 'POKE', 'POKEL',
  'POKEW', 'PRINT', 'PRIO', 'RANDOMIZE', 'ROL', 'ROR', 'SAVE', 'SCROLL', 'SOUND', 'SPRITE',
  'SPRITE.A', 'STOP', 'SYSTEM', 'TEXT', 'TINT', 'TRACE', 'TRACK', 'VOLUME', 'WAIT', 'WINDOW',
];

const FUNCTIONS = [
  'ABS', 'ASC', 'ATAN', 'BIN$', 'CEIL', 'CELL.A', 'CELL.C', 'CHR$', 'CLAMP', 'COLOR', 'COS',
  'CURSOR.X', 'CURSOR.Y', 'EASE', 'EXP', 'FILE$', 'FLOOR', 'FSIZE', 'HEX$', 'HIT', 'INKEY$',
  'INSTR', 'INT', 'KEYBOARD', 'LEFT$', 'LEN', 'LERP', 'LOG', 'MAX', 'MCELL.A', 'MCELL.C', 'MID$',
  'MIN', 'MUSIC', 'PEEK', 'PEEKL', 'PEEKW', 'PI', 'RASTER', 'RIGHT$', 'RND', 'ROM', 'SAFE.B',
  'SAFE.L', 'SAFE.R', 'SAFE.T', 'SCROLL.X', 'SCROLL.Y', 'SGN', 'SHOWN.H', 'SHOWN.W', 'SIN', 'SIZE',
  'SPRITE.A', 'SPRITE.C', 'SPRITE.X', 'SPRITE.Y', 'SQR', 'STR$', 'TAN', 'TAP', 'TIMER', 'TOUCH',
  'TOUCH.CHANGE', 'TOUCH.DRAG', 'TOUCH.LONG', 'TOUCH.PX', 'TOUCH.PY', 'TOUCH.TAP', 'TOUCH.X',
  'TOUCH.Y', 'UBOUND', 'VAL', 'WINDOW.H', 'WINDOW.W', 'WINDOW.X', 'WINDOW.Y',
];

// Words that only qualify another instruction: `WAIT VBL`, `BG FILL … CHAR c`, `ON THUMBNAIL CALL`.
// `LET` is still tokenized, so it is reserved, but no instruction accepts it.
const MODIFIERS = ['AT', 'CHAR', 'LET', 'SOURCE', 'THUMBNAIL', 'VBL', 'VIEW', 'WAVE'];

const SUBSTRING_TARGETS = ['LEFT$', 'MID$', 'RIGHT$'];

// Any word that is not a function may qualify a command (`SPRITE 0 PAL 1`, `SPRITE OFF 0 TO 9`).
// `SPRITE` is excluded because, in an expression, it starts `SPRITE HIT(…)`.
const ARGUMENT_WORDS = [
  ...new Set([...STATEMENTS, ...COMMANDS, ...MODIFIERS, 'OFF', 'ON', 'STEP', 'TO']),
].filter((word) => !FUNCTIONS.includes(word) && word !== 'SPRITE');

const PREC = {
  or: 1,
  and: 2,
  not: 3,
  compare: 4,
  add: 5,
  mod: 6,
  mul: 7,
  unary: 8,
  pow: 9,
  call: 10,
};

checkKeywordsAgainstEngine();

module.exports = grammar({
  name: 'rmx',

  word: ($) => $.identifier,

  extras: ($) => [/[ \t]+/, $.comment],

  rules: {
    source_file: ($) => seq(optional($._lines), optional($._statement), optional($.rom_section)),

    _lines: ($) => repeat1(choice($._terminator, $.label, seq($._statement, $._terminator))),

    block: ($) => $._lines,

    _terminator: ($) => choice($._newline, ':'),

    _newline: (_) => /\r?\n|\r/,

    comment: (_) => token(seq("'", /[^\r\n]*/)),

    label: ($) => seq(field('name', $.identifier), token.immediate(':')),

    _statement: ($) => choice(
      $._simple_statement,
      $.if_statement,
      $.for_statement,
      $.do_statement,
      $.while_statement,
      $.repeat_statement,
      $.sub_definition,
    ),

    _simple_statement: ($) => choice(
      $.inline_if_statement,
      $.assignment,
      $.command,
      $.goto_statement,
      $.gosub_statement,
      $.return_statement,
      $.restore_statement,
      $.call_statement,
      $.on_statement,
      $.end_statement,
      $.exit_statement,
    ),

    if_statement: ($) => seq(
      keyword('IF'),
      field('condition', $._expression),
      keyword('THEN'),
      $._terminator,
      optional(field('body', $.block)),
      repeat($.else_if_clause),
      optional($.else_clause),
      words('END IF'),
    ),

    else_if_clause: ($) => seq(
      keyword('ELSE'),
      keyword('IF'),
      field('condition', $._expression),
      keyword('THEN'),
      $._terminator,
      optional(field('body', $.block)),
    ),

    else_clause: ($) => seq(keyword('ELSE'), $._terminator, optional(field('body', $.block))),

    inline_if_statement: ($) => prec.right(seq(
      keyword('IF'),
      field('condition', $._expression),
      keyword('THEN'),
      field('consequence', $._simple_statement),
      optional(seq(keyword('ELSE'), field('alternative', $._simple_statement))),
    )),

    for_statement: ($) => seq(
      keyword('FOR'),
      field('variable', $._scalar),
      '=',
      field('start', $._expression),
      keyword('TO'),
      field('end', $._expression),
      optional(seq(keyword('STEP'), field('step', $._expression))),
      $._terminator,
      optional(field('body', $.block)),
      keyword('NEXT'),
      optional(field('variable', $._scalar)),
    ),

    do_statement: ($) => seq(
      keyword('DO'),
      $._terminator,
      optional(field('body', $.block)),
      keyword('LOOP'),
    ),

    while_statement: ($) => seq(
      keyword('WHILE'),
      field('condition', $._expression),
      $._terminator,
      optional(field('body', $.block)),
      keyword('WEND'),
    ),

    repeat_statement: ($) => seq(
      keyword('REPEAT'),
      $._terminator,
      optional(field('body', $.block)),
      keyword('UNTIL'),
      field('condition', $._expression),
    ),

    sub_definition: ($) => seq(
      keyword('SUB'),
      field('name', $.identifier),
      optional(field('parameters', $.parameter_list)),
      $._terminator,
      optional(field('body', $.block)),
      words('END SUB'),
    ),

    parameter_list: ($) => seq('(', commaSep1($.parameter), ')'),

    parameter: ($) => seq(field('name', $._scalar), optional(seq('(', ')'))),

    assignment: ($) => seq(
      field('left', choice($._scalar, $.array_element, alias($._substring_target, $.builtin_call))),
      '=',
      field('right', $._expression),
    ),

    _substring_target: ($) => seq(
      field('name', choice(...SUBSTRING_TARGETS.map(keyword))),
      field('arguments', $.argument_list),
    ),

    command: ($) => prec.right(seq(
      choice(
        field('keyword', choice(...STATEMENTS.map(keyword))),
        field('name', choice(...COMMANDS.map(keyword))),
      ),
      repeat(choice($._expression, $.tuple, ',', ';', $.modifier)),
    )),

    modifier: (_) => choice(...ARGUMENT_WORDS.map(keyword)),

    tuple: ($) => seq(
      '(',
      optional($._expression),
      repeat1(seq(',', optional($._expression))),
      ')',
    ),

    goto_statement: ($) => seq(keyword('GOTO'), field('label', $.label_ref)),

    gosub_statement: ($) => seq(keyword('GOSUB'), field('label', $.label_ref)),

    return_statement: ($) => seq(keyword('RETURN'), optional(field('label', $.label_ref))),

    restore_statement: ($) => seq(keyword('RESTORE'), optional(field('label', $.label_ref))),

    call_statement: ($) => seq(
      keyword('CALL'),
      field('name', $.identifier),
      optional(field('arguments', $.argument_list)),
    ),

    on_statement: ($) => seq(
      keyword('ON'),
      field('value', choice($._expression, $.modifier)),
      choice(
        seq(
          choice(keyword('GOTO'), keyword('GOSUB'), keyword('RESTORE')),
          commaSep1(field('label', $.label_ref)),
        ),
        seq(
          keyword('CALL'),
          optional(field('arguments', $.argument_list)),
          commaSep1(field('procedure', $.identifier)),
        ),
        keyword('OFF'),
      ),
    ),

    end_statement: (_) => keyword('END'),

    exit_statement: (_) => seq(keyword('EXIT'), optional(keyword('SUB'))),

    label_ref: ($) => $.identifier,

    _expression: ($) => choice(
      $.number,
      $.string,
      $._scalar,
      $.array_element,
      $.builtin_call,
      $.parenthesized_expression,
      $.unary_expression,
      $.binary_expression,
    ),

    _scalar: ($) => choice($.identifier, $.string_identifier),

    array_element: ($) => prec(PREC.call, seq(
      field('name', $._scalar),
      field('arguments', $.argument_list),
    )),

    builtin_call: ($) => choice(
      prec.right(PREC.call, seq(
        field('name', choice(
          ...FUNCTIONS.filter((word) => word !== 'RND').map(keyword),
          seq(keyword('SPRITE'), keyword('HIT')),
        )),
        optional(field('arguments', $.argument_list)),
      )),
      prec.right(PREC.call, seq(
        field('name', keyword('RND')),
        optional(field('arguments', $.argument_list)),
        optional(field('address', choice($.number, $._scalar, $.array_element))),
      )),
    ),

    argument_list: ($) => seq(
      '(',
      optional($._argument),
      repeat(seq(',', optional($._argument))),
      ')',
    ),

    _argument: ($) => choice($._expression, $.range),

    range: ($) => seq(field('start', $._expression), keyword('TO'), field('end', $._expression)),

    parenthesized_expression: ($) => seq('(', $._expression, ')'),

    unary_expression: ($) => choice(
      prec(PREC.not, seq(field('operator', keyword('NOT')), field('operand', $._expression))),
      prec(PREC.unary, seq(field('operator', choice('-', '+')), field('operand', $._expression))),
    ),

    binary_expression: ($) => {
      const table = [
        [PREC.or, choice(keyword('OR'), keyword('XOR'))],
        [PREC.and, keyword('AND')],
        [PREC.compare, choice('=', '<>', '<', '>', '<=', '>=')],
        [PREC.add, choice('+', '-')],
        [PREC.mod, keyword('MOD')],
        [PREC.mul, choice('*', '/', '\\')],
        [PREC.pow, '^'],
      ];
      return choice(...table.map(([precedence, operator]) => prec.left(precedence, seq(
        field('left', $._expression),
        field('operator', operator),
        field('right', $._expression),
      ))));
    },

    number: (_) => token(choice(/\d+(\.\d*)?/, /\$[0-9A-Fa-f]*/, /%[01]*/)),

    string: (_) => /"[^"\r\n]*"/,

    identifier: (_) => /[A-Za-z_][A-Za-z0-9_]*/,

    string_identifier: (_) => /[A-Za-z_][A-Za-z0-9_]*\$/,

    rom_section: ($) => repeat1($.rom_entry),

    rom_entry: ($) => seq(
      '#',
      field('slot', $.rom_slot),
      ':',
      optional(field('comment', $.rom_comment)),
      repeat(choice($._newline, $.rom_data)),
    ),

    rom_slot: (_) => /\d+/,

    rom_comment: (_) => /[^\r\n]+/,

    rom_data: (_) => /[0-9A-Fa-f]+/,
  },
});

/**
 * A case-insensitive keyword, named after its upper-case spelling.
 *
 * Words holding `.` or `$` cannot be found by keyword extraction, so they get a lexical
 * precedence that beats `identifier`/`string_identifier` (`LEFT$` vs a `left$` variable).
 *
 * @param {string} word
 * @returns {AliasRule}
 */
function keyword(word) {
  const pattern = caseInsensitive(word);
  return alias(/[.$]/.test(word) ? token(prec(1, pattern)) : pattern, word);
}

/**
 * Two keywords lexed as one token, so that `END IF` closing a block never competes with the
 * `END` instruction starting a statement.
 *
 * @param {string} phrase
 * @returns {AliasRule}
 */
function words(phrase) {
  const source = phrase.split(' ').map((word) => caseInsensitive(word).source).join('[ \\t]+');
  return alias(token(prec(1, new RegExp(source))), phrase);
}

/**
 * @param {string} word
 * @returns {RegExp}
 */
function caseInsensitive(word) {
  return new RegExp([...word]
    .map((c) => (/[A-Z]/.test(c) ? `[${c}${c.toLowerCase()}]` : `\\${c}`))
    .join(''));
}

/**
 * @param {RuleOrLiteral} rule
 * @returns {SeqRule}
 */
function commaSep1(rule) {
  return seq(rule, repeat(seq(',', rule)));
}

function checkKeywordsAgainstEngine() {
  const tokenFile = path.join(__dirname, '../../backend.core/interpreter/token.c');
  const engine = new Set(
    [...fs.readFileSync(tokenFile, 'utf8').matchAll(/^\s*"([A-Z][A-Z0-9.$]*)",/gm)].map((m) => m[1]),
  );
  const known = new Set([...STRUCTURE, ...OPERATORS, ...STATEMENTS, ...COMMANDS, ...FUNCTIONS, ...MODIFIERS]);
  const missing = [...engine].filter((word) => !known.has(word));
  const stale = [...known].filter((word) => !engine.has(word));
  if (missing.length > 0 || stale.length > 0) {
    throw new Error(
      `grammar.js is out of sync with ${tokenFile}\n` +
      `  unclassified engine keywords: ${missing.join(' ') || '-'}\n` +
      `  keywords the engine no longer has: ${stale.join(' ') || '-'}`,
    );
  }
}

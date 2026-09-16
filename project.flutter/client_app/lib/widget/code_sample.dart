import 'package:flutter/material.dart';

import 'package:lowresrmx/style.dart';

const Key codeSampleKey = Key('code-sample');
const Key codeSampleTextKey = Key('code-sample-text');

const double codeSampleHeight = 176.0;

class _Token {
  final String? scope;
  final String text;

  const _Token(this.scope, this.text);
}

const List<_Token> _sample = [
  _Token('comment', "' FONT PREVIEW\n"),
  _Token('keyword', 'CLS'),
  _Token(null, '\n'),
  _Token('keyword', 'FOR'),
  _Token(null, ' I='),
  _Token('number', '1'),
  _Token(null, ' '),
  _Token('keyword', 'TO'),
  _Token(null, ' '),
  _Token('number', '4'),
  _Token(null, '\n'),
  _Token(null, ' '),
  _Token('keyword', 'PRINT'),
  _Token(null, ' '),
  _Token('string', '"RMX"'),
  _Token(null, ';I\n'),
  _Token('keyword', 'NEXT'),
  _Token(null, ' I'),
];

class MyCodeSample extends StatelessWidget {
  final double fontSize;
  final bool bigFinger;

  const MyCodeSample(
      {required this.fontSize, required this.bigFinger, super.key});

  @override
  Widget build(BuildContext context) {
    final Map<String, TextStyle> theme =
        Theme.of(context).brightness == Brightness.dark
            ? styleDark
            : styleLight;
    final TextStyle base = TextStyle(
      fontFamily: 'RecursiveLinear',
      fontSize: fontSize,
      height: 1.2,
      color: theme['root']!.color,
    );
    return Container(
      key: codeSampleKey,
      height: codeSampleHeight,
      padding: EdgeInsets.fromLTRB(fontSize * bigFingerPaddingFactor, 12.0,
          bigFinger ? fontSize * bigFingerPaddingFactor : 12.0, 12.0),
      decoration: BoxDecoration(
        color: Theme.of(context).colorScheme.surfaceContainerHighest,
        borderRadius: BorderRadius.circular(8.0),
      ),
      child: SingleChildScrollView(
        scrollDirection: Axis.horizontal,
        child: Text.rich(
          TextSpan(children: [
            for (final _Token t in _sample)
              TextSpan(
                  text: t.text, style: t.scope == null ? null : theme[t.scope])
          ]),
          key: codeSampleTextKey,
          style: base,
          softWrap: false,
          textScaler: TextScaler.noScaling,
        ),
      ),
    );
  }
}

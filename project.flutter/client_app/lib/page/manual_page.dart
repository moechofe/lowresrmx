import 'dart:io' show Platform;

import 'package:flutter/material.dart';
import 'package:flutter/services.dart' show rootBundle;

import 'package:url_launcher/url_launcher.dart';
import 'package:webview_flutter/webview_flutter.dart';

const String _manualAsset = 'asset/manual.html';

final Uri _manualWebsite = Uri.parse('https://ret.ro.it/documentation.html');

bool get _hasWebView =>
    Platform.isAndroid || Platform.isIOS || Platform.isMacOS;

class MyManualTile extends StatelessWidget {
  const MyManualTile({super.key});

  @override
  Widget build(BuildContext context) {
    return ListTile(
      leading: const Icon(Icons.menu_book_rounded),
      title: Text(_hasWebView ? "Manual (offline)" : "Manual (online)"),
      onTap: () async {
        Navigator.of(context).pop();
        if (_hasWebView) {
          Navigator.of(context).push(
            MaterialPageRoute(builder: (context) => const MyManualPage()),
          );
        } else if (!await launchUrl(_manualWebsite,
            mode: LaunchMode.externalApplication)) {
          throw Exception('Could not launch $_manualWebsite');
        }
      },
    );
  }
}

class _ManualHeading {
  final int level;
  final String id;
  final String title;

  final bool hasKeywords;

  const _ManualHeading(this.level, this.id, this.title,
      {this.hasKeywords = false});
}

final RegExp _headingPattern =
    RegExp(r'<h([234])([^>]*)>(.*?)</h\1>', dotAll: true);
final RegExp _idPattern = RegExp(r'id="([^"]+)"');
final RegExp _tagPattern = RegExp(r'<[^>]*>');

String _unescape(String text) => text
    .replaceAll('&lt;', '<')
    .replaceAll('&gt;', '>')
    .replaceAll('&quot;', '"')
    .replaceAll('&#39;', "'")
    .replaceAll('&nbsp;', ' ')
    .replaceAll('&amp;', '&');

List<_ManualHeading> _parseHeadings(String html) {
  final List<RegExpMatch> matches = _headingPattern.allMatches(html).toList();
  final List<_ManualHeading> headings = [];
  for (int i = 0; i < matches.length; i++) {
    final int level = int.parse(matches[i].group(1)!);
    if (level == 4) continue;
    final RegExpMatch? id = _idPattern.firstMatch(matches[i].group(2)!);
    if (id == null) continue;
    headings.add(_ManualHeading(
      level,
      id.group(1)!,
      _unescape(matches[i].group(3)!.replaceAll(_tagPattern, '')).trim(),
      hasKeywords: level == 3 &&
          i + 1 < matches.length &&
          matches[i + 1].group(1) == '4',
    ));
  }
  return headings;
}

class MyManualPage extends StatefulWidget {
  const MyManualPage({super.key});

  @override
  State<MyManualPage> createState() => _MyManualPageState();
}

class _MyManualPageState extends State<MyManualPage> {
  late final WebViewController _controller;
  late final Future<List<_ManualHeading>> _headings =
      rootBundle.loadString(_manualAsset).then(_parseHeadings);

  bool _loaded = false;
  String? _pendingAnchor;

  @override
  void initState() {
    super.initState();
    _controller = WebViewController()
      ..setJavaScriptMode(JavaScriptMode.unrestricted)
      ..setNavigationDelegate(NavigationDelegate(
        onPageFinished: (_) {
          _loaded = true;
          final String? anchor = _pendingAnchor;
          _pendingAnchor = null;
          if (anchor != null) _jumpTo(anchor);
        },
        onNavigationRequest: (NavigationRequest request) {
          if (request.url.startsWith('http://') ||
              request.url.startsWith('https://')) {
            launchUrl(Uri.parse(request.url),
                mode: LaunchMode.externalApplication);
            return NavigationDecision.prevent;
          }
          return NavigationDecision.navigate;
        },
      ))
      ..loadFlutterAsset(_manualAsset);
  }

  void _jumpTo(String id) {
    if (!_loaded) {
      _pendingAnchor = id;
      return;
    }
    _controller
        .runJavaScript("document.getElementById('$id').scrollIntoView(true);");
  }

  Widget _buildToc({required bool closeDrawer}) {
    return FutureBuilder<List<_ManualHeading>>(
      future: _headings,
      builder: (context, snapshot) {
        if (!snapshot.hasData) return const SizedBox.shrink();
        final List<_ManualHeading> headings = snapshot.data!;
        return ListView.builder(
          itemCount: headings.length,
          itemBuilder: (context, index) {
            final _ManualHeading heading = headings[index];
            return ListTile(
              // dense: true,
              contentPadding: EdgeInsets.only(
                  left: heading.level == 2 ? 16.0 : 32.0, right: 8.0),
              leading:
                  heading.hasKeywords ? const Icon(Icons.menu_rounded) : null,
              minLeadingWidth: 0,
              title: Text(
                heading.title,
              ),
              onTap: () {
                if (closeDrawer) Navigator.of(context).pop();
                _jumpTo(heading.id);
              },
            );
          },
        );
      },
    );
  }

  @override
  Widget build(BuildContext context) {
    final bool isLargeEnough = MediaQuery.of(context).size.width > 600;
    final Widget web = WebViewWidget(controller: _controller);

    if (isLargeEnough) {
      return Scaffold(
        appBar: AppBar(title: const Text("Manual")),
        body: Row(children: [
          SizedBox(width: 300, child: _buildToc(closeDrawer: false)),
          Expanded(child: web),
        ]),
      );
    }

    return Scaffold(
      appBar: AppBar(title: const Text("Manual")),
      drawer: Drawer(
        child: Column(children: [
          AppBar(
            leading: IconButton(
              icon: const Icon(Icons.arrow_back),
              onPressed: () => Navigator.of(context)
                ..pop()
                ..pop(),
            ),
            title: const Text("Back"),
          ),
          Expanded(child: _buildToc(closeDrawer: true)),
        ]),
      ),
      body: web,
    );
  }
}

import 'package:flutter/material.dart';
import 'package:flutter/services.dart' show rootBundle;

import 'package:markdown_widget/markdown_widget.dart';

class MyManualTile extends StatelessWidget {
  const MyManualTile({
    super.key,
  });

  @override
  Widget build(BuildContext context) {
    return ListTile(
      leading: const Icon(Icons.menu_book_rounded),
      title: const Text("LowresRMX Manual"),
      onTap: () {
        Navigator.of(context).pop();
        Navigator.of(context).push(
          MaterialPageRoute(builder: (context) => const MyManualPage()),
        );
      },
    );
  }
}

// final MarkdownConfig config = MarkdownConfig(configs: [
//   H1Config()..padding = const EdgeInsets.symmetric(horizontal: 16.0),
//   H2Config()..padding = const EdgeInsets.symmetric(horizontal: 16.0),
//   H3Config()..padding = const EdgeInsets.symmetric(horizontal: 16.0),
// 	PConfig()
// ]);

class MyManualPage extends StatefulWidget {
  const MyManualPage({super.key});

  @override
  State<MyManualPage> createState() => _MyManualPageState();
}

class _MyManualPageState extends State<MyManualPage> {
  // Need to be recreated or if not the TOC will not appear.
  late TocController tocController = TocController();

  Widget _buildToc() {
    return TocWidget(controller: tocController);
  }

  Widget _buildUsingDrawer(BuildContext context, String manualData) {
    return Scaffold(
      appBar: AppBar(
        title: const Text("Manual"),
      ),
      drawer: Drawer(
          child: Column(children: [
        AppBar(
          leading: IconButton(
            icon: const Icon(Icons.arrow_back),
            onPressed: () {
              Navigator.of(context)
                ..pop() // Drawer
                ..pop(); // MyManualPage
            },
          ),
          title: const Text("Table of Contents"),
        ),
        Expanded(child: _buildToc())
      ])),
      // Need to pass a UniqueKey or the TOC link will not link to the correct section.
      body: MarkdownWidget(
      	padding: const EdgeInsets.all(16.0),
      	tocController: tocController,
      	data: manualData,
      	key: UniqueKey(),
      	// config: config,
      ),
    );
  }

  Widget _buildUsingRow(String manualData) {
    return Scaffold(
        appBar: AppBar(
          leading: IconButton(
            icon: const Icon(Icons.arrow_back),
            onPressed: () {
              Navigator.of(context).pop();
            },
          ),
          title: const Text("Manual"),
        ),
        body: Row(children: [
          SizedBox(
            width: 300,
            child: _buildToc(),
          ),
          Expanded(
            child:
                // Need to pass a UniqueKey or the TOC link will not link to the correct section.
                MarkdownWidget(
                    tocController: tocController,
                    data: manualData,
                    key: UniqueKey()),
          )
        ]));
  }

  @override
  Widget build(BuildContext context) {
    tocController = TocController();
    final Size screenSize = MediaQuery.of(context).size;
    bool isLargeEnough = screenSize.width > 600;
    return FutureBuilder(
        future: rootBundle.loadString('asset/manual.md'),
        builder: (BuildContext context, AsyncSnapshot<String> snapshot) {
          if (snapshot.hasData) {
            if (isLargeEnough) {
              return _buildUsingRow(snapshot.data!);
            } else {
              return _buildUsingDrawer(context, snapshot.data!);
            }
          } else if (snapshot.hasError) {
            return const Text('Error loading manual');
          } else {
            return const Center(
              child: Column(
                mainAxisAlignment: MainAxisAlignment.center,
                children: [
                  CircularProgressIndicator(),
                ],
              ),
            );
          }
        });
  }
}

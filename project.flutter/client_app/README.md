# lowresrmx

A new Flutter project.

## Getting Started

This project is a starting point for a Flutter application.

A few resources to get you started if this is your first Flutter project:

- [Lab: Write your first Flutter app](https://docs.flutter.dev/get-started/codelab)
- [Cookbook: Useful Flutter samples](https://docs.flutter.dev/cookbook)

For help getting started with Flutter development, view the
[online documentation](https://docs.flutter.dev/), which offers tutorials,
samples, guidance on mobile development, and a full API reference.

I manually added a namespace for sw_rend in the .pub-cache folder

I manually fixed the missing macro in core_plugin/src/core.c

    #ifndef SL_GLOBMATCH_NEGATE
    #define SL_GLOBMATCH_NEGATE '^'       /* std char set negation char */
    #endif

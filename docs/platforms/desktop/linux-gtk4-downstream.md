# Linux GTK4 Downstream Compatibility

GTK3 and GTK4 cannot be loaded into the same process. Selecting the GTK4
runner therefore requires every native Linux plugin in an application to build
against GTK4 as well. A successful engine build alone does not establish that
an existing application is GTK4-compatible.

## Development Baseline

This implementation builds on the dual-runner work in
[flutter/flutter#186594](https://github.com/flutter/flutter/pull/186594). Keep
the contributing branch available as a Git remote or fetched ref so fixes can
be synchronized without losing commit attribution. GTK3 remains the default;
GTK4 is an explicit project or command-line choice.

The older GTK4 work in
[flutter/engine#50960](https://github.com/flutter/engine/pull/50960) and
[flutter/engine#53369](https://github.com/flutter/engine/pull/53369) is useful
design history, but it is not a second implementation to merge. The current
dual-runner branch already incorporates the same direction and has continued
through current engine, accessibility, renderer, and tooling changes.

## Plugin Requirements

A GTK4-compatible Linux plugin must:

1. link only to `gtk4`, never `gtk+-3.0`;
2. compile against the GTK4 Flutter runner and public headers;
3. avoid GTK3-only widget, event, window, monitor, and GDK APIs;
4. propagate `FLUTTER_LINUX_GTK4` to native sources that support both variants;
5. fail at configure time instead of silently linking GTK3 into a GTK4 app; and
6. test both variants if the same package continues to support GTK3.

The generated GTK4 plugin template demonstrates the expected CMake and source
layout. Existing plugin packages still require individual ports.

## Zuko And Vixen Acceptance Graph

Zuko and Vixen are useful downstream acceptance applications because they use
Flutter rendering heavily while sharing the same Yaru desktop integration
stack.

| Component | GTK4 status | Required work |
|---|---|---|
| `flterm` / `libghostty` | GTK-independent | Validate native assets and rendering with the GTK4 runner. |
| Yaru Flutter themes and widgets | Mostly Dart | Keep the Dart UI; port its transitive Linux window plugins. |
| `gtk` | GTK3 native plugin | Add a dual GTK3/GTK4 implementation. |
| `screen_retriever_linux` | GTK3 native plugin | Port monitor enumeration and notifications to GTK4. |
| `window_manager` | GTK3 native plugin | Port window lookup, state, movement, sizing, and signal handling. |
| `yaru_window_linux` | GTK3 native plugin | Port title-bar/window controls or replace its native operations with the GTK4 window-manager path. |
| `url_launcher_linux` | GTK4 fork exists | Start from [`richyo-codes/packages`](https://github.com/richyo-codes/packages/tree/feat-add-gtk4-support-url-launcher-v2/packages/url_launcher/url_launcher_linux) and synchronize it with the current package release. |
| `flutter_secure_storage_linux` | No direct GTK UI | Compile-test its GLib/libsecret and Flutter registrar usage with GTK4. |
| Zuko native transport and Vixen FFI | GTK-independent | Validate their native-asset build hooks and bundled libraries. |

The GTK4 desktop-drop and lifecycle work in
[`richyo-codes/flutter-plugins`](https://github.com/richyo-codes/flutter-plugins/tree/desktop-linux-gtk4)
is a useful plugin-porting example, but it does not cover the Yaru dependency
graph above.

## Existing Runners

Do not only change `pkg_check_modules` in an existing GTK3 runner. GTK4 removed
or replaced APIs used by the standard GTK3 runner, including container child
attachment, widget show/realize flow, top-level lookup, header-bar properties,
and several event and window APIs.

Generate a GTK4 application as the reference implementation:

```bash
flutter create --platforms=linux --linux-gtk=gtk4 gtk4_reference
```

Then port application-specific runner code onto that generated runner. Zuko
needs to retain its Impeller setup. Vixen additionally needs to retain its
Wayland check, method channel, and `FlPixelBufferTexture` lifecycle. Compare the
result with the generated GTK4 runner rather than incrementally replacing GTK3
calls without a reference.

Commit the project default once the native plugin graph is compatible:

```yaml
flutter:
  config:
    linux-gtk-default: gtk4
```

During migration, keep GTK3 and GTK4 output directories separate and build both
variants where practical:

```bash
flutter build linux --linux-gtk=gtk3
flutter build linux --linux-gtk=gtk4
```

## Distribution Validation

Use Flutter's downloaded engine sysroot for engine compilation. A generic
distribution container does not replace that sysroot. Use downstream packaging
environments for the application-level checks:

- the pinned Freedesktop build environment for Zuko;
- the pinned GNOME/Flatpak SDK for Vixen; and
- a native Wayland session for input, IME, window-state, accessibility, and
  renderer smoke tests.

For Flatpak validation, confirm with `ldd` or the equivalent manifest inspection
that the final process closure contains `libgtk-4.so.1` and does not contain
`libgtk-3.so.0`. This is a release gate because loading both GTK major versions
in one process is unsupported.

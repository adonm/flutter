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

The generated Linux plugin template links the selected GTK interface exported
by the application runner. Existing plugin packages still require individual
ports.

## Zuko And Vixen Acceptance Graph

Zuko and Vixen are useful downstream acceptance applications because they use
Flutter rendering heavily while sharing the same Yaru desktop integration
stack.

| Component | GTK4 status | Required work |
|---|---|---|
| `flterm` / `libghostty` | GTK-independent | Validate native assets and rendering with the GTK4 runner. |
| Yaru Flutter themes and widgets | Mostly Dart | Keep the Dart UI; port its transitive Linux window plugins. |
| `gtk` | Dual GTK validation ref | Detect the process GTK major and use correctly typed GTK3/GTK4 icon lookup. |
| `screen_retriever_linux` | Dual GTK validation ref | Validate X11 monitor/cursor behavior and explicit Wayland limitations. |
| `window_manager` | Dual GTK validation ref | Validate GTK4 capability errors, events, sizing, and lifecycle handling. |
| `yaru_window_linux` | Dual GTK validation ref | Validate GTK4 title-bar state, CSS, input, and toplevel lifecycle. |
| `url_launcher_linux` | Dual GTK validation ref | Preserve parent launch context and synchronous error reporting on GTK4. |
| `flutter_secure_storage_linux` | No direct GTK UI | Compile-test its GLib/libsecret and Flutter registrar usage with GTK4. |
| Zuko native transport and Vixen FFI | GTK-independent | Validate their native-asset build hooks and bundled libraries. |

The GTK4 desktop-drop and lifecycle work in
[`richyo-codes/flutter-plugins`](https://github.com/richyo-codes/flutter-plugins/tree/desktop-linux-gtk4)
is a useful plugin-porting example, but it does not cover the Yaru dependency
graph above.

### Reproducible Validation Refs

The following staging refs are pinned by the Zuko and Vixen validation
branches. Each plugin branch starts at the listed upstream head and retains a
GTK3 fallback when `FLUTTER_LINUX_GTK4` is not defined. These are validation
refs for this Flutter change, not published plugin releases or substitutes for
upstream review.

| Component | Upstream base | Validation ref |
|---|---|---|
| `gtk` | [`canonical/gtk.dart@0734034`](https://github.com/canonical/gtk.dart/commit/0734034c1191a8df274800cbc7f8ab521757f108) | [`adonm/gtk.dart@8f9a5db`](https://github.com/adonm/gtk.dart/commit/8f9a5db741176a25657fbf6fecb2a90df8741e48) |
| `screen_retriever_linux` | [`leanflutter/screen_retriever@5fc2b57`](https://github.com/leanflutter/screen_retriever/commit/5fc2b573292e559edb4b2bb74680db8e6c0ca65f) | [`adonm/screen_retriever@a4d8f71`](https://github.com/adonm/screen_retriever/commit/a4d8f718ecbee78207945aa87ded0f97e2509c87) |
| `window_manager` | [`leanflutter/window_manager@4080b59`](https://github.com/leanflutter/window_manager/commit/4080b59c0b1084c6085f1613c660382cd164ad76) | [`adonm/window_manager@3acfbdd`](https://github.com/adonm/window_manager/commit/3acfbdd1624202edfe03ccc7d4eae548b6c18459) |
| `yaru_window_linux` | [`ubuntu/yaru_window.dart@27ec2b1`](https://github.com/ubuntu/yaru_window.dart/commit/27ec2b14bb9bd180d2b979a91efaefb8afd86814) | [`adonm/yaru_window.dart@4f22de3`](https://github.com/adonm/yaru_window.dart/commit/4f22de320ea0270662506900e1ae070eb7b6cb13) |
| `url_launcher_linux` | [`flutter/packages@9f95026`](https://github.com/flutter/packages/commit/9f950268cf0fa09c4fd914a6d389e90e3aa9d791) | [`adonm/packages@0ab7672`](https://github.com/adonm/packages/commit/0ab767292efb16fadbfec3c7c119380343b4fbd8) |
| Zuko | [`adonm/zuko@19a5aa4`](https://github.com/adonm/zuko/commit/19a5aa4260b1dfbb5a7a90d2a197bb8a3948712a) | [`adonm/zuko@0109bad`](https://github.com/adonm/zuko/commit/0109bada14b21e939bd3aa817b57f9b7cbf6121e) |
| Vixen | [`adonm/vixen@7fc7219`](https://github.com/adonm/vixen/commit/7fc721991912b74ebea10a325e5682a4990a52a8) | [`adonm/vixen@275b598`](https://github.com/adonm/vixen/commit/275b598d60ef853ffbc740024d7113f4c6e66660) |

The application refs select GTK4 as their project default and use immutable
Git dependency overrides for the plugin refs. Zuko's GTK4 bundle includes
`libghostty` and its Iroh native asset. Vixen's GTK4 bundle includes its Rust
FFI library and custom Wayland texture runner.

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

- the pinned GNOME 50 build and Flatpak environment for Zuko;
- the pinned GNOME build environment for Vixen; and
- a native Wayland session for input, IME, window-state, accessibility, and
  renderer smoke tests.

For Flatpak validation, confirm with `ldd` or the equivalent manifest inspection
that the final process closure contains `libgtk-4.so.1` and does not contain
`libgtk-3.so.0`. This is a release gate because loading both GTK major versions
in one process is unsupported.

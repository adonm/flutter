// Copyright 2014 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:ffi';

import 'package:file/file.dart';
import 'package:file_testing/file_testing.dart';
import 'package:flutter_tools/src/base/io.dart';
import 'package:flutter_tools/src/cache.dart';

import '../commands.shard/permeable/utils/project_testing_utils.dart';
import '../src/common.dart';
import 'test_utils.dart';

void main() {
  late Directory tempDir;
  late Directory projectRoot;

  setUpAll(() async {
    Cache.disableLocking();
    await ensureFlutterToolsSnapshot();
    tempDir = createResolvedTempDirectorySync('create_linux_gtk4_build_test.');
    await _runFlutterSnapshot(<String>[
      'create',
      '--platforms=linux',
      '--linux-gtk=gtk4',
      'hello',
    ], tempDir);
    projectRoot = tempDir.childDirectory('hello');
  });

  tearDownAll(() async {
    tryToDelete(tempDir);
    await restoreFlutterToolsSnapshot();
  });

  test(
    'flutter create generates a Linux project that builds GTK4 and GTK3',
    () async {
      final File cmakeFile = projectRoot.childDirectory('linux').childFile('CMakeLists.txt');
      expect(cmakeFile, exists);
      expect(
        cmakeFile.readAsStringSync(),
        allOf(
          contains('set(LINUX_GTK_PKG gtk4)'),
          contains(r'pkg_check_modules(GTK REQUIRED IMPORTED_TARGET ${LINUX_GTK_PKG})'),
        ),
      );
      expect(
        projectRoot.childFile('pubspec.yaml').readAsStringSync(),
        contains('linux-gtk-default: gtk4'),
      );

      await _runFlutterSnapshot(<String>['build', 'linux', '--debug', '--no-pub'], projectRoot);

      final arch = Abi.current() == Abi.linuxArm64 ? 'arm64' : 'x64';
      final File executable = fileSystem.file(
        fileSystem.path.join(
          projectRoot.path,
          'build',
          'linux-gtk4',
          arch,
          'debug',
          'bundle',
          'hello',
        ),
      );
      expect(executable, exists);

      await _runFlutterSnapshot(<String>[
        'build',
        'linux',
        '--debug',
        '--no-pub',
        '--linux-gtk=gtk3',
      ], projectRoot);
      expect(
        fileSystem.file(
          fileSystem.path.join(
            projectRoot.path,
            'build',
            'linux',
            arch,
            'debug',
            'bundle',
            'hello',
          ),
        ),
        exists,
      );
    },
    skip: !platform.isLinux, // [intended] Linux builds only work on Linux.
  );
}

Future<void> _runFlutterSnapshot(List<String> flutterCommandArgs, Directory workingDir) async {
  final String flutterRoot = fileSystem.path.normalize(
    fileSystem.path.join(fileSystem.currentDirectory.path, '..', '..'),
  );
  final String dartBinary = fileSystem.path.join(
    flutterRoot,
    'bin',
    'cache',
    'dart-sdk',
    'bin',
    platform.isWindows ? 'dart.exe' : 'dart',
  );
  final String flutterToolsSnapshotPath = fileSystem.path.join(
    flutterRoot,
    'bin',
    'cache',
    'flutter_tools.snapshot',
  );

  final args = <String>[
    dartBinary,
    flutterToolsSnapshotPath,
    ...getLocalEngineArguments(),
    ...flutterCommandArgs,
  ];

  final ProcessResult exec = await processManager.run(args, workingDirectory: workingDir.path);
  expect(exec, const ProcessResultMatcher());
}

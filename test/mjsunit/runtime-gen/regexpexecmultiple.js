// Copyright 2014 the V8 project authors. All rights reserved.
// AUTO-GENERATED BY tools/generate-runtime-tests.py, DO NOT MODIFY
// Flags: --allow-natives-syntax --harmony --harmony-proxies
var _regexp = /ab/g;
var _subject = "foo";
var arg2 = ['a'];
var arg3 = ['a'];
%RegExpExecMultiple(_regexp, _subject, arg2, arg3);

$ErrorActionPreference = 'Stop'
$source = Get-Content -Raw -LiteralPath (Join-Path $PSScriptRoot '../cpp/nectar_rpc.cpp')
if ($source -match '(?:get_enumerator|move_next|current)\([^;\n]*,\s*nullptr\)') {
    throw 'An iterator invocation passes null MethodInfo; v152 generic-shared iterators require rgctx.'
}
foreach ($call in @(
    'get_enumerator(enumerable, get_enumerator_method)',
    'move_next(enumerator, move_next_method)',
    'get_enumerator(enumerable, get_impl)',
    'move_next(enumerator, move_impl)',
    'current(enumerator, current_impl)'
)) {
    if (!$source.Contains($call)) { throw "Missing resolved-MethodInfo call: $call" }
}
Write-Output 'PASS: all five iterator call sites carry their resolved implementation MethodInfo (static regression guard).'

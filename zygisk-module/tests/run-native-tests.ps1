param(
    [Parameter(Mandatory=$true)][string]$Device,
    [string]$NdkPath = 'C:\Users\Ody\AppData\Local\CodexTools\android-ndk\android-ndk-r27d',
    [string]$Adb = 'C:\Users\Ody\AppData\Local\CodexTools\android-sdk\platform-tools\adb.exe'
)
$ErrorActionPreference = 'Stop'
$stage = Join-Path ([IO.Path]::GetTempPath()) ('pikmin-policy-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $stage | Out-Null
Copy-Item -LiteralPath $PSScriptRoot -Destination (Join-Path $stage 'tests') -Recurse
Copy-Item -LiteralPath (Join-Path (Split-Path $PSScriptRoot -Parent) 'cpp') -Destination (Join-Path $stage 'cpp') -Recurse
$clang = Join-Path $NdkPath 'toolchains/llvm/prebuilt/windows-x86_64/bin/aarch64-linux-android28-clang++.cmd'
$remote = '/data/local/tmp/' + (Split-Path $stage -Leaf)
& $Adb -s $Device shell mkdir -m 700 $remote
if ($LASTEXITCODE -ne 0) { throw 'Cannot create isolated test directory' }
foreach ($test in Get-ChildItem (Join-Path $stage 'tests') -Filter '*_test.cpp') {
    $binary = Join-Path $stage $test.BaseName
    & $clang -std=c++20 -O2 -static-libstdc++ $test.FullName -o $binary
    if ($LASTEXITCODE -ne 0) { throw ('Compile failed: ' + $test.Name) }
    & $Adb -s $Device push $binary "$remote/$($test.BaseName)"
    if ($LASTEXITCODE -ne 0) { throw 'Test push failed' }
    & $Adb -s $Device shell chmod 700 "$remote/$($test.BaseName)"
    & $Adb -s $Device shell "$remote/$($test.BaseName)"
    if ($LASTEXITCODE -ne 0) { throw ('Test failed: ' + $test.Name) }
}
Write-Output 'PASS: native policy executables only; game process and module not modified.'

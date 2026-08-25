[CmdletBinding()]
param(
    [string]$NdkPath = "$env:LOCALAPPDATA\CodexTools\android-ndk\android-ndk-r27d",
    [string]$CmakePath = "$env:LOCALAPPDATA\CodexTools\android-build-tools\cmake\data\bin\cmake.exe",
    [string]$NinjaPath = "$env:LOCALAPPDATA\CodexTools\android-build-tools\bin\ninja.exe"
)

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$module = Join-Path $root 'module'
$toolchain = Join-Path $NdkPath 'build\cmake\android.toolchain.cmake'
$stage = Join-Path ([System.IO.Path]::GetTempPath()) ("pikmin-nectar-build-" + [guid]::NewGuid().ToString('N'))
$source = Join-Path $stage 'cpp'
$build = Join-Path $stage 'build\arm64-v8a'
New-Item -ItemType Directory -Force -Path $stage | Out-Null
Copy-Item -LiteralPath (Join-Path $root 'cpp') -Destination $stage -Recurse

& $CmakePath -G Ninja "-DCMAKE_MAKE_PROGRAM=$NinjaPath" `
    "-DCMAKE_TOOLCHAIN_FILE=$toolchain" -DANDROID_ABI=arm64-v8a `
    -DANDROID_PLATFORM=android-28 -DCMAKE_BUILD_TYPE=Release -S $source -B $build
if ($LASTEXITCODE -ne 0) { throw 'CMake configuration failed.' }
& $CmakePath --build $build
if ($LASTEXITCODE -ne 0) { throw 'Native build failed.' }

$zygisk = Join-Path $module 'zygisk'
New-Item -ItemType Directory -Force -Path $zygisk | Out-Null
Copy-Item -LiteralPath (Join-Path $build 'libpikmin_nectar_rpc.so') `
    -Destination (Join-Path $zygisk 'arm64-v8a.so') -Force

$zip = Join-Path $root 'pikmin-nectar-rpc-v151.zip'
if (Test-Path -LiteralPath $zip) { Remove-Item -LiteralPath $zip -Force }
& tar.exe -a -c -f $zip -C $module module.prop service.sh zygisk
if ($LASTEXITCODE -ne 0) { throw 'Module packaging failed.' }
Get-Item -LiteralPath $zip | Select-Object FullName, Length

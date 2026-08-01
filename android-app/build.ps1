[CmdletBinding()]
param(
    [string]$SdkPath = '',
    [string]$JavaHome = ''
)

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$tools = Join-Path (Split-Path -Parent $root) '.tools\android'
$sdk = if ($SdkPath) { $SdkPath } else { Join-Path $tools 'sdk' }
$javaHome = if ($JavaHome) { $JavaHome } else { (Get-ChildItem (Join-Path $tools 'jdk') -Directory | Select-Object -First 1).FullName }
$buildTools = Join-Path $sdk 'build-tools\35.0.0'
$androidJar = Join-Path $sdk 'platforms\android-35\android.jar'
$out = Join-Path $root 'build'
$classes = Join-Path $out 'classes'
$dex = Join-Path $out 'dex'

if (Test-Path $out) { Remove-Item -LiteralPath $out -Recurse -Force }
New-Item -ItemType Directory -Force -Path $classes, $dex | Out-Null

$env:JAVA_HOME = $javaHome
& (Join-Path $javaHome 'bin\javac.exe') -encoding UTF-8 -source 8 -target 8 `
    -classpath $androidJar -d $classes `
    (Join-Path $root 'src\dev\ody\pikminnectar\MainActivity.java')
if ($LASTEXITCODE -ne 0) { throw 'javac failed' }

$unsigned = Join-Path $out 'nectar-control-unsigned.apk'
& (Join-Path $buildTools 'aapt2.exe') link -I $androidJar `
    --manifest (Join-Path $root 'AndroidManifest.xml') -o $unsigned
if ($LASTEXITCODE -ne 0) { throw 'aapt2 link failed' }

$classesJar = Join-Path $out 'classes.jar'
& (Join-Path $javaHome 'bin\jar.exe') cf $classesJar -C $classes .
if ($LASTEXITCODE -ne 0) { throw 'classes jar failed' }
& (Join-Path $buildTools 'd8.bat') --lib $androidJar --min-api 28 --output $dex $classesJar
if ($LASTEXITCODE -ne 0) { throw 'd8 failed' }
Push-Location $dex
try { & (Join-Path $javaHome 'bin\jar.exe') uf $unsigned classes.dex }
finally { Pop-Location }
if ($LASTEXITCODE -ne 0) { throw 'adding classes.dex failed' }

$aligned = Join-Path $out 'nectar-control-aligned.apk'
& (Join-Path $buildTools 'zipalign.exe') -f 4 $unsigned $aligned
if ($LASTEXITCODE -ne 0) { throw 'zipalign failed' }

$keystore = Join-Path $root 'debug.keystore'
if (!(Test-Path $keystore)) {
    & (Join-Path $javaHome 'bin\keytool.exe') -genkeypair -v -keystore $keystore `
        -storepass android -alias androiddebugkey -keypass android -keyalg RSA -keysize 2048 `
        -validity 10000 -dname 'CN=Android Debug,O=Android,C=US'
    if ($LASTEXITCODE -ne 0) { throw 'keytool failed' }
}

$apk = Join-Path $root 'pikmin-nectar-control.apk'
& (Join-Path $buildTools 'apksigner.bat') sign --ks $keystore --ks-pass pass:android `
    --key-pass pass:android --out $apk $aligned
if ($LASTEXITCODE -ne 0) { throw 'apksigner failed' }
& (Join-Path $buildTools 'apksigner.bat') verify --verbose $apk
if ($LASTEXITCODE -ne 0) { throw 'APK verification failed' }
Get-Item $apk | Select-Object FullName, Length

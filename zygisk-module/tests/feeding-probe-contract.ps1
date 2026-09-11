$ErrorActionPreference = 'Stop'
$source=Get-Content -Raw -LiteralPath (Join-Path $PSScriptRoot '../cpp/feeding_probe.inc')
foreach($required in @('observe-only','pikmin-feeding-probe.txt','mode != "observe"','count > 512','color > 4','ScopedManagedRoot','0x7297C4C','0x7281428','0x5F753A0','0x5F753B0','0x5F753C0')) {
    if(!$source.Contains($required)){throw "Missing probe guard: $required"}
}
if($source -match 'SendFeed|SendPick|object_new|reinterpret_cast<.*\(.*proto') { throw 'Readonly feeding probe must not construct or send game requests' }
if($source -match '\\tready\\t') { throw 'Unvalidated catalogue must never announce ready' }
Write-Output 'PASS: feeding probe opt-in, metadata pins, bounded rooted reads and observe-only contract (not device validation)'

$ErrorActionPreference = "Stop"
$root = Split-Path $PSScriptRoot -Parent
$out = Join-Path $root "dist/CRIS-VOX-COLOR-Windows-x64"
$vst = Join-Path $root "build/CrisVoxColor_artefacts/Release/VST3/CRIS VOX COLOR.vst3"
$exe = Join-Path $root "build/CrisVoxColor_artefacts/Release/Standalone/CRIS VOX COLOR.exe"
if (!(Test-Path $vst)) { throw "No VST3 found: build Release first." }
New-Item -ItemType Directory -Force "$out/VST3", "$out/Standalone", "$out/licenses" | Out-Null
Copy-Item -Recurse -Force $vst "$out/VST3/"
Copy-Item -Force $exe "$out/Standalone/"
Copy-Item "$root/LICENSE", "$root/THIRD_PARTY.md", "$root/INSTALAR_EN_FL_STUDIO.md" $out
Copy-Item "$root/licenses/*" "$out/licenses/"
Get-ChildItem $out -File -Recurse | ForEach-Object {
  $relative = [IO.Path]::GetRelativePath($out, $_.FullName)
  "{0}  {1}" -f (Get-FileHash $_.FullName -Algorithm SHA256).Hash.ToLower(), $relative
} | Set-Content "$out/SHA256SUMS.txt"
Write-Host "Package ready: $out"

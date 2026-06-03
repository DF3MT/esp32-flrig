#Requires -Version 5.1
<#
.SYNOPSIS
  Windows-Einrichtung für FT8/WSJT-X über ESP32 CAT Panel (WiFi).

.DESCRIPTION
  - Python-Abhängigkeiten (sounddevice, numpy)
  - VB-Audio Virtual Cable prüfen / per winget installieren
  - Audio-Geräte für ft8_remote.py ermitteln
  - Optional CAT-Test und Audio-Bridge starten

.EXAMPLE
  .\scripts\ft8_windows_setup.ps1
  .\scripts\ft8_windows_setup.ps1 -EspHost 192.168.4.1 -StartBridge
  .\scripts\ft8_windows_setup.ps1 -InstallVbCable -ListOnly
#>
param(
    [string]$EspHost = "192.168.4.1",
    [int]$PlayDevice = -1,
    [int]$RecDevice = -1,
    [int]$EspRate = 48000,
    [switch]$InstallVbCable,
    [switch]$SkipAudioDriver,
    [switch]$StartBridge,
    [switch]$ListOnly,
    [switch]$TestCat,
    [switch]$UseCableB
)

$ErrorActionPreference = "Stop"
$Repo = Split-Path -Parent $PSScriptRoot
$Ft8Script = Join-Path $Repo "scripts\ft8_remote.py"
$ReqFile = Join-Path $Repo "scripts\requirements-ft8.txt"
$VbCableUrl = "https://vb-audio.com/Cable/"
$WingetPackageIds = @(
    "VB-Audio.VBCABLE",
    "VB-Audio.VirtualCable",
    "VB-Audio.Cable"
)

function Write-Step([string]$Msg) { Write-Host "`n==> $Msg" -ForegroundColor Cyan }
function Write-Ok([string]$Msg)   { Write-Host "    $Msg" -ForegroundColor Green }
function Write-Warn([string]$Msg) { Write-Host "    $Msg" -ForegroundColor Yellow }

function Get-PythonExe {
    foreach ($c in @("python", "py", "python3")) {
        $cmd = Get-Command $c -ErrorAction SilentlyContinue
        if ($cmd) {
            $ver = & $cmd.Source -c "import sys; print(f'{sys.version_info.major}.{sys.version_info.minor}')" 2>$null
            if ($LASTEXITCODE -eq 0) { return $cmd.Source }
        }
    }
    return $null
}

function Install-PythonDeps([string]$Python) {
    Write-Step "Python-Abhängigkeiten"
    & $Python -m pip install --upgrade pip -q
    if (Test-Path $ReqFile) {
        & $Python -m pip install -r $ReqFile -q
    } else {
        & $Python -m pip install sounddevice numpy -q
    }
    Write-Ok "sounddevice, numpy installiert"
}

function Test-VbAudioPresent {
    $patterns = @("VB-Audio", "CABLE Input", "CABLE Output", "CABLE-A", "CABLE-B")
    $audio = Get-CimInstance Win32_SoundDevice -ErrorAction SilentlyContinue
    if ($audio) {
        foreach ($a in $audio) {
            foreach ($p in $patterns) {
                if ($a.Name -like "*$p*") { return $true }
            }
        }
    }
    # Fallback: Geräteliste aus Python
    $devs = Get-AudioDevicesViaPython (Get-PythonExe)
    foreach ($d in $devs) {
        foreach ($p in $patterns) {
            if ($d.Name -like "*$p*") { return $true }
        }
    }
    return $false
}

function Install-VbAudioCable {
    Write-Step "VB-Audio Virtual Cable"
    $winget = Get-Command winget -ErrorAction SilentlyContinue
    if (-not $winget) {
        Write-Warn "winget nicht gefunden."
        Write-Host "    Manuell installieren: $VbCableUrl"
        Write-Host "    Nach Installation PC neu starten (empfohlen), Skript erneut ausführen."
        Start-Process $VbCableUrl
        return $false
    }
    foreach ($id in $WingetPackageIds) {
        Write-Host "    Versuche winget install $id ..."
        $proc = Start-Process -FilePath "winget" -ArgumentList @(
            "install", "--id", $id, "-e", "--accept-package-agreements", "--accept-source-agreements"
        ) -Wait -PassThru -NoNewWindow
        if ($proc.ExitCode -eq 0) {
            Write-Ok "VB-Audio über winget installiert ($id)"
            Write-Warn "Bei Treiber-Setup-Dialog: Installation bestätigen, ggf. Neustart."
            return $true
        }
    }
    Write-Warn "Automatische Installation fehlgeschlagen."
    Write-Host "    Bitte manuell: $VbCableUrl (VBCABLE_Setup_x64.exe)"
    Start-Process $VbCableUrl
    return $false
}

function Get-AudioDevicesViaPython([string]$Python) {
    $code = @'
import sounddevice as sd
for i, d in enumerate(sd.query_devices()):
    print(f"{i}\t{d['name']}\t{d['max_input_channels']}\t{d['max_output_channels']}")
'@
    $lines = & $Python -c $code 2>$null
    if ($LASTEXITCODE -ne 0) { return @() }
    $list = @()
    foreach ($line in $lines) {
        if ($line -match '^\s*(\d+)\s+(.+?)\s+(\d+)\s+(\d+)\s*$') {
            $list += [PSCustomObject]@{
                Index = [int]$Matches[1]
                Name = $Matches[2].Trim()
                InCh = [int]$Matches[3]
                OutCh = [int]$Matches[4]
            }
        }
    }
    return $list
}

function Resolve-VbCableDeviceIds([array]$Devices, [bool]$PreferB) {
    # ft8_remote: play = Playback (Radio-RX zu WSJT-X Input-Kette)
    #             rec  = Recording (WSJT-X Output abgreifen)
    #
    # VB-Audio:  Playback "CABLE Input"  →  Recording "CABLE Output" (Monitor)
    # WSJT-X:    Input  = "CABLE Output"     Output = "CABLE Input"
    $playName = if ($PreferB) { "CABLE-B Input" } else { "CABLE Input" }
    $recName  = if ($PreferB) { "CABLE-B Output" } else { "CABLE Output" }

    $play = $Devices | Where-Object { $_.OutCh -gt 0 -and $_.Name -like "*$playName*" } | Select-Object -First 1
    $rec  = $Devices | Where-Object { $_.InCh -gt 0 -and $_.Name -like "*$recName*" } | Select-Object -First 1

    if (-not $play -and -not $PreferB) {
        $play = $Devices | Where-Object { $_.OutCh -gt 0 -and $_.Name -like "*CABLE*Input*" } | Select-Object -First 1
    }
    if (-not $rec -and -not $PreferB) {
        $rec = $Devices | Where-Object { $_.InCh -gt 0 -and $_.Name -like "*CABLE*Output*" } | Select-Object -First 1
    }

    return @{
        Play = if ($play) { $play.Index } else { -1 }
        Rec  = if ($rec) { $rec.Index } else { -1 }
        PlayName = if ($play) { $play.Name } else { "" }
        RecName  = if ($rec) { $rec.Name } else { "" }
    }
}

function Show-AudioDeviceList([array]$Devices) {
    Write-Step "Audio-Geräte (sounddevice Index)"
    foreach ($d in $Devices) {
        $io = "IN:$($d.InCh) OUT:$($d.OutCh)"
        Write-Host ("  [{0,2}] {1,-50} {2}" -f $d.Index, $d.Name, $io)
    }
}

function Show-WsjtxGuide([string]$HostIp, [int]$PlayIdx, [int]$RecIdx, [string]$PlayName, [string]$RecName) {
    Write-Step "WSJT-X Einstellungen"
    @"

  Settings → Radio:
    Rig:            Hamlib NET rigctl
    Rig model:      2
    Host:           $HostIp
    Port:           4532
    PTT method:     CAT
    Mode:           USB
    Split Operation: OFF

  Settings → Audio:
    Input device:   $RecName  (VB: „CABLE Output“ – hört Funk-RX vom Kabel)
    Output device:  $PlayName (VB: „CABLE Input“ – WSJT-X TX ins Kabel)
    Sample rate:    48000 Hz (Windows: Einstellungen → System → Sound → Erweitert)

  Reihenfolge:
    1) Dieses Skript mit -StartBridge (oder ft8_remote.py manuell)
    2) Dann WSJT-X starten

  Hinweis Namensgebung VB-Audio:
    ft8_remote --play-device  spielt Funk-RX auf "$PlayName"
    WSJT-X Input muss "$RecName" sein (hört das Kabel)
    WSJT-X Output muss "$PlayName" sein
    ft8_remote --rec-device   nimmt von "$RecName" auf

"@ | Write-Host
}

function Test-EspCat([string]$HostIp) {
    Write-Step "CAT-Test (rigctl)"
    $rigctl = Get-Command rigctl -ErrorAction SilentlyContinue
    if (-not $rigctl) {
        $wsl = Get-Command wsl.exe -ErrorAction SilentlyContinue
        if ($wsl) {
            Write-Host "    rigctl nicht in Windows PATH – teste via WSL ..."
            $out = wsl -e bash -lc "rigctl -m 2 -r ${HostIp}:4532 f 2>/dev/null" 2>$null
            if ($out) {
                Write-Ok "Frequenz: $out"
                return $true
            }
        }
        Write-Warn "rigctl nicht gefunden. Install: choco install hamlib ODER WSL: sudo apt install hamlib-tools"
        Write-Host "    Manuell: rigctl -m 2 -r ${HostIp}:4532 f"
        return $false
    }
    $freq = & rigctl -m 2 -r "${HostIp}:4532" f 2>$null
    if ($LASTEXITCODE -eq 0) {
        Write-Ok "Frequenz: $freq"
        return $true
    }
    Write-Warn "Keine Antwort von ${HostIp}:4532 – ESP erreichbar? CAT aktiv?"
    return $false
}

function Start-Ft8Bridge([string]$Python, [string]$HostIp, [int]$Play, [int]$Rec, [int]$Rate) {
    Write-Step "Starte ft8_remote.py (Fenster offen lassen)"
    $args = @(
        $Ft8Script, $HostIp,
        "--play-device", $Play,
        "--rec-device", $Rec,
        "--esp-rate", $Rate
    )
    Write-Host "    python $($args -join ' ')"
    Start-Process -FilePath $Python -ArgumentList $args -WorkingDirectory $Repo
}

# ── Main ───────────────────────────────────────────────────────────────────
Write-Host @"

 ESP32 CAT Panel – FT8 Setup (Windows)
 ESP: $EspHost  |  Audio 48 kHz  |  CAT :4532

"@ -ForegroundColor White

$Python = Get-PythonExe
if (-not $Python) {
    throw "Python nicht gefunden. Installiere Python 3.10+ von https://www.python.org/downloads/ (Haken: Add to PATH)"
}
Write-Ok "Python: $Python"

Install-PythonDeps $Python

if (-not $SkipAudioDriver) {
    if (-not (Test-VbAudioPresent)) {
        Write-Warn "VB-Audio Virtual Cable nicht erkannt."
        if ($InstallVbCable -or (Read-Host "VB-Audio jetzt installieren? [J/n]") -match '^(|j|y|yes|ja)$') {
            Install-VbAudioCable | Out-Null
            Write-Warn "Nach der Installation Setup ggf. neu starten und Skript erneut ausführen."
        }
    } else {
        Write-Ok "VB-Audio (oder virtuelles Kabel) erkannt"
    }
} else {
    Write-Warn "Audio-Treiber-Check übersprungen (-SkipAudioDriver)"
}

$devices = Get-AudioDevicesViaPython $Python
if ($devices.Count -eq 0) {
    throw "Keine Audio-Geräte – sounddevice / Treiber prüfen"
}

Show-AudioDeviceList $devices

if ($ListOnly) { exit 0 }

$resolved = Resolve-VbCableDeviceIds $devices $UseCableB
if ($PlayDevice -lt 0) { $PlayDevice = $resolved.Play }
if ($RecDevice -lt 0) { $RecDevice = $resolved.Rec }

if ($PlayDevice -lt 0 -or $RecDevice -lt 0) {
    Write-Warn "VB-Cable nicht eindeutig – bitte Indizes manuell wählen:"
    Write-Host "  .\scripts\ft8_windows_setup.ps1 -EspHost $EspHost -PlayDevice <OUT-Index> -RecDevice <IN-Index> -StartBridge"
    Write-Host "  Oder zwei Kabel: VB-Cable + VB-Cable A/B von https://vb-audio.com/Cable/"
    exit 1
}

Write-Ok "play-device (Funk-RX abspielen): [$PlayDevice] $($resolved.PlayName)"
Write-Ok "rec-device  (WSJT-X TX aufnehmen): [$RecDevice] $($resolved.RecName)"

Show-WsjtxGuide $EspHost $PlayDevice $RecDevice $resolved.PlayName $resolved.RecName

if ($TestCat) { Test-EspCat $EspHost | Out-Null }

if ($StartBridge) {
    Start-Ft8Bridge $Python $EspHost $PlayDevice $RecDevice $EspRate
    Write-Ok "Audio-Bridge gestartet. Jetzt WSJT-X öffnen."
} else {
    Write-Host "`nBridge starten mit:"
    Write-Host "  .\scripts\ft8_windows_setup.ps1 -EspHost $EspHost -PlayDevice $PlayDevice -RecDevice $RecDevice -StartBridge" -ForegroundColor Yellow
}

Write-Host "`nAlternativ (plattformübergreifend):" -ForegroundColor DarkGray
Write-Host "  copy scripts\ft8_config.example.json %USERPROFILE%\.config\esp32-flrig\ft8.json"
Write-Host "  $Python scripts\ft8_setup.py --config scripts\ft8_config.example.json --start"

param(
    [string]$BuildDirectory = "cmake-build-debug",
    [string]$MapsDirectory = "cmake-build-debug\stage10-maps-items",
    [string]$CMake = "",
    [string]$NSIS = ""
)

$ErrorActionPreference = "Stop"
$root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path

function Resolve-ProjectPath([string]$path) {
    if ([System.IO.Path]::IsPathRooted($path)) { return $path }
    return Join-Path $root $path
}

$build = Resolve-ProjectPath $BuildDirectory
$mapSource = Resolve-ProjectPath $MapsDirectory
$cache = Join-Path $build "CMakeCache.txt"
$distribution = Join-Path $root "dist"
$stage = Join-Path $distribution "CircuitoEscape"
$installer = Join-Path $distribution "CircuitoEscape-Setup.exe"

if (-not (Test-Path -LiteralPath $cache)) {
    throw "CMake build directory not configured: $build"
}

if (-not $CMake) {
    $command = Get-Command cmake.exe -ErrorAction SilentlyContinue
    if ($command) { $CMake = $command.Source }
}
if (-not $CMake -or -not (Test-Path -LiteralPath $CMake)) {
    throw "Pass -CMake with the path to cmake.exe"
}

function Read-CacheValue([string]$name) {
    $entry = Select-String -LiteralPath $cache -Pattern "^$([regex]::Escape($name)):[^=]*=(.*)$" | Select-Object -First 1
    if (-not $entry) { throw "Missing $name in CMakeCache.txt" }
    return $entry.Matches[0].Groups[1].Value.Replace("/", "\")
}

$compiler = Read-CacheValue "CMAKE_CXX_COMPILER"
$compilerDirectory = Split-Path $compiler -Parent
$objdump = Join-Path $compilerDirectory "objdump.exe"
if (-not (Test-Path -LiteralPath $objdump)) {
    throw "MinGW objdump.exe not found: $objdump"
}

foreach ($name in @("mapa_facil.txt", "mapa_medio.txt", "mapa_dificil.txt")) {
    $file = Join-Path $mapSource $name
    if (-not (Test-Path -LiteralPath $file) -or (Get-Item -LiteralPath $file).Length -eq 0) {
        throw "Missing or empty validated map: $file"
    }
    $firstLine = Get-Content -LiteralPath $file -TotalCount 1
    if ($firstLine -ne "CIRCUIT_ESCAPE_MAP 1") {
        throw "Invalid map file signature: $file"
    }
}

& $CMake -S $root -B $build
if ($LASTEXITCODE -ne 0) { throw "CMake configuration failed" }
& $CMake --build $build --target map_generator escape_circuit
if ($LASTEXITCODE -ne 0) { throw "CMake build failed" }

New-Item -ItemType Directory -Path $distribution -Force | Out-Null
if (Test-Path -LiteralPath $stage) { Remove-Item -LiteralPath $stage -Recurse -Force }
& $CMake --install $build --prefix $stage --component Runtime
if ($LASTEXITCODE -ne 0) { throw "CMake installation failed" }

$maps = Join-Path $stage "assets\maps"
New-Item -ItemType Directory -Path $maps -Force | Out-Null
foreach ($name in @("mapa_facil.txt", "mapa_medio.txt", "mapa_dificil.txt")) {
    Copy-Item -LiteralPath (Join-Path $mapSource $name) -Destination (Join-Path $maps $name) -Force
}

$binaryDirectory = Join-Path $stage "bin"
$executables = @("map_generator.exe", "escape_circuit.exe")
foreach ($name in $executables) {
    if (-not (Test-Path -LiteralPath (Join-Path $binaryDirectory $name))) {
        throw "Missing installed executable: $name"
    }
}

$queue = [System.Collections.Queue]::new()
foreach ($name in $executables) { $queue.Enqueue((Join-Path $binaryDirectory $name)) }
$checked = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
$copied = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)

while ($queue.Count -gt 0) {
    $binary = [string]$queue.Dequeue()
    if (-not $checked.Add($binary)) { continue }
    $imports = & $objdump -p $binary
    if ($LASTEXITCODE -ne 0) { throw "Unable to inspect DLL imports of $binary" }
    foreach ($line in $imports) {
        if ($line -notmatch '^\s*DLL Name:\s*(\S+\.dll)\s*$') { continue }
        $name = $Matches[1]
        if ($name -match '^(api-ms-win-|ext-ms-win-)') { continue }
        $fromCompiler = Join-Path $compilerDirectory $name
        $fromExecutable = Join-Path (Split-Path $binary -Parent) $name
        if (Test-Path -LiteralPath $fromCompiler) {
            if ($copied.Add($name)) {
                $destination = Join-Path $binaryDirectory $name
                Copy-Item -LiteralPath $fromCompiler -Destination $destination -Force
                $queue.Enqueue($destination)
                Write-Output "Bundled DLL: $name"
            }
        } elseif (Test-Path -LiteralPath $fromExecutable) {
            $queue.Enqueue($fromExecutable)
        } elseif (Test-Path -LiteralPath (Join-Path $env:WINDIR "System32\$name")) {
            continue
        } else {
            throw "Unresolved runtime DLL: $name (required by $binary)"
        }
    }
}

$previousPath = $env:Path
try {
    # Run with only the staged binaries and Windows system directories.
    # Otherwise the compiler directory could mask a missing packaged DLL.
    $env:Path = "$binaryDirectory;$env:WINDIR\System32;$env:WINDIR"
    foreach ($name in $executables) {
        & (Join-Path $binaryDirectory $name) --help | Out-Null
        if ($LASTEXITCODE -ne 0) { throw "Staged $name failed its startup check" }
    }
} finally {
    $env:Path = $previousPath
}

if (-not $NSIS) {
    $command = Get-Command makensis.exe -ErrorAction SilentlyContinue
    if ($command) { $NSIS = $command.Source }
    else {
        $locations = @(
            "${env:ProgramFiles(x86)}\NSIS\makensis.exe",
            "$env:ProgramFiles\NSIS\makensis.exe",
            "$env:LOCALAPPDATA\Programs\NSIS\makensis.exe"
        )
        $NSIS = $locations | Where-Object { Test-Path -LiteralPath $_ } | Select-Object -First 1
    }
}
if (-not $NSIS -or -not (Test-Path -LiteralPath $NSIS)) {
    throw "NSIS (makensis.exe) is required to create the installer. Prepared files: $stage"
}

& $NSIS "/DSTAGING=$stage" "/DOUTPUT=$installer" (Join-Path $PSScriptRoot "CircuitEscape.nsi")
if ($LASTEXITCODE -ne 0) { throw "NSIS installer creation failed" }
if (-not (Test-Path -LiteralPath $installer)) { throw "Installer not produced: $installer" }
Write-Output "Installer created: $installer"

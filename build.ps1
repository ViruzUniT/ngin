$operation = $args[0]
$type = ""

$dxcVersion = "1.9.2609"
$dxcArchiveUrl = "https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.9.2609/dxc_2026_09_29.zip"
$dxcArchiveHash = "ad31b1fc8443175d204f77a611fdb3ef2ec42759bdc2f1167368de24a4a7e7f1"
$dxcRoot = Join-Path $PSScriptRoot ".tools/dxc"
$dxcBin = Join-Path $dxcRoot "$dxcVersion/bin/x64"
$dxcExe = Join-Path $dxcBin "dxc.exe"

function Ensure-Dxc {
  if (Test-Path $dxcExe) {
    return
  }

  Write-Host "DXC $dxcVersion not found. Downloading the official DirectX Shader Compiler..." -ForegroundColor Cyan
  $archive = Join-Path $dxcRoot "dxc-$dxcVersion.zip"
  $installDir = Join-Path $dxcRoot $dxcVersion
  $stagingDir = Join-Path $dxcRoot "$dxcVersion-extract"
  New-Item -ItemType Directory -Force -Path $dxcRoot | Out-Null

  try {
    Invoke-WebRequest -Uri $dxcArchiveUrl -OutFile $archive
    $actualHash = (Get-FileHash -Path $archive -Algorithm SHA256).Hash.ToLowerInvariant()
    if ($actualHash -ne $dxcArchiveHash) {
      throw "DXC archive checksum mismatch. Expected $dxcArchiveHash, got $actualHash"
    }

    if (Test-Path $stagingDir) {
      Remove-Item -LiteralPath $stagingDir -Recurse -Force
    }
    Expand-Archive -LiteralPath $archive -DestinationPath $stagingDir -Force
    $stagedExe = Join-Path $stagingDir "bin/x64/dxc.exe"
    if (-not (Test-Path $stagedExe)) {
      throw "The downloaded DXC archive does not contain bin/x64/dxc.exe"
    }

    if (Test-Path $installDir) {
      Remove-Item -LiteralPath $installDir -Recurse -Force
    }
    Move-Item -LiteralPath $stagingDir -Destination $installDir
  }
  catch {
    Write-Host "DXC setup failed: $_" -ForegroundColor Red
    exit 1
  }
  finally {
    Remove-Item -LiteralPath $archive -Force -ErrorAction SilentlyContinue
    Remove-Item -LiteralPath $stagingDir -Recurse -Force -ErrorAction SilentlyContinue
  }

  if (-not (Test-Path $dxcExe)) {
    Write-Host "DXC setup did not produce $dxcExe" -ForegroundColor Red
    exit 1
  }
}

if($operation) { } 
else {
  Write-Host "You need to pass an operation type:" -ForegroundColor Red
  Write-Host "build.ps1 (gen|compdb|build|run|clean) [vs2022|vs2026|gmake|DEBUG|STAGING|RELEASE]" -ForegroundColor Cyan
  exit 1
}

if($operation -eq "gen") {
  Ensure-Dxc
  if (-not (Get-Command premake5 -ErrorAction SilentlyContinue)) {
      Write-Host "premake5 not found on PATH`nPlease install premake5 on your system" -ForegroundColor Red
      exit 1
  }

  if($args.Count -lt 2) {
    $type = "gmake"
  } else {
    $type = $args[1]
  }

  premake5 $type 
  if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
  }
}
elseif($operation -eq "compdb") {
  if(-not (Get-Command make -ErrorAction SilentlyContinue)) {
    Write-Host "make not found on PATH`nInstall GNU Make or use the project's configured toolchain" -ForegroundColor Red
    exit 1
  }
  if(-not (Get-Command compiledb -ErrorAction SilentlyContinue)) {
    Write-Host "compiledb not found on PATH`nInstall it with: pip install compiledb" -ForegroundColor Red
    exit 1
  }

  if($args.Count -lt 2) { $type = "DEBUG" } else { $type = $args[1].ToUpper() }
  if(($type -ne "DEBUG") -and ($type -ne "STAGING") -and ($type -ne "RELEASE")) {
    Write-Host "Invalid compile database configuration: $type" -ForegroundColor Red
    Write-Host "Use DEBUG, STAGING or RELEASE" -ForegroundColor Cyan
    exit 1
  }

  if(-not (Test-Path "build/Ngin.make") -or -not (Test-Path "build/Sandbox.make")) {
    Write-Host "Generated GNU Make files not found. Run build.ps1 gen gmake first" -ForegroundColor Red
    exit 1
  }

  $makeConfig = $type.ToLower()
  $buildDir = (Resolve-Path "build").Path
  $log = [IO.Path]::GetTempFileName()
  Push-Location "build"
  try {
    & make -f Ngin.make config=$makeConfig -n -B > $log
    if($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    & make -f Sandbox.make config=$makeConfig -n -B >> $log
    if($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
  }
  finally {
    Pop-Location
  }

  & compiledb -f -p $log -o compile_commands.json -d $buildDir
  $result = $LASTEXITCODE
  Remove-Item $log -Force -ErrorAction SilentlyContinue
  if($result -ne 0) { exit $result }

  $database = Get-Content "compile_commands.json" -Raw | ConvertFrom-Json
  foreach($entry in $database) {
    for($index = 0; $index -lt ($entry.arguments.Count - 1); $index++) {
      if(($entry.arguments[$index] -eq "-include") -and ($entry.arguments[$index + 1] -match "bin-int/.*/pch\.h$")) {
        $entry.arguments[$index + 1] = "../src/ngin/include/ngin/pch.h"
      }
    }
  }
  $database | ConvertTo-Json -Depth 10 | Set-Content "compile_commands.json" -Encoding utf8
}
elseif($operation -eq "build") {
  Ensure-Dxc
  if($args.Count -lt 2) {
    $type = "DEBUG"
  } else {
    $type = $args[1].ToUpper()
  }

  if(($type -ne "DEBUG") -and ($type -ne "STAGING") -and ($type -ne "RELEASE")) {
    Write-Host "Invalid build type: $type" -ForegroundColor Red
    Write-Host "Use DEBUG, STAGING or RELEASE" -ForegroundColor Cyan
    exit 1
  }

  $makeConfig = $type.ToLower()
  $generated = (Test-Path "build/Makefile") -or (Test-Path "build/Ngin.sln")
  $built = $false

  if((Test-Path "C:/mingw64/bin/g++.exe") -and (-not (Get-Command g++ -ErrorAction SilentlyContinue))) {
    $env:PATH = "C:/mingw64/bin;$env:PATH"
  }

  if(Test-Path "build/Makefile") {
    if(Get-Command make -ErrorAction SilentlyContinue) {
      make SHELL=cmd.exe -C build config=$makeConfig
      $built = $true
    }
    elseif(Get-Command mingw32-make -ErrorAction SilentlyContinue) {
      mingw32-make -C build config=$makeConfig
      $built = $true
    }
  }

  if((-not $built) -and (Test-Path "build/Ngin.sln")) {
    if(-not (Get-Command msbuild -ErrorAction SilentlyContinue)) {
      Write-Host "make or msbuild not found on PATH" -ForegroundColor Red
      exit 1
    }

    msbuild "build/Ngin.sln" /p:Configuration=$type /p:Platform=x64
    $built = $true
  }

  if(-not $built) {
    if($generated) {
      Write-Host "make or msbuild not found on PATH" -ForegroundColor Red
      exit 1
    }

    Write-Host "No generated build files found. Run build.ps1 gen first" -ForegroundColor Red
    exit 1
  }

  if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
  }

}
elseif($operation -eq "run") {
  if($args.Count -lt 2) {
    $type = "DEBUG"
  } else {
    $type = $args[1].ToUpper()
  }

  if(($type -ne "DEBUG") -and ($type -ne "STAGING") -and ($type -ne "RELEASE")) {
    Write-Host "Invalid build type: $type" -ForegroundColor Red
    Write-Host "Use DEBUG, STAGING or RELEASE" -ForegroundColor Cyan
    exit 1
  }

  & $PSCommandPath build $type
  if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
  }

  $sandbox = "bin/$type-windows-x86_64/Sandbox/Sandbox.exe"

  if(-not (Test-Path $sandbox)) {
    Write-Host "Sandbox executable not found: $sandbox" -ForegroundColor Red
    exit 1
  }

  & $sandbox
  if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
  }
}
elseif($operation -eq "clean") {
  if(Test-Path "build") {
    Remove-Item "build" -Recurse -Force
  }

  if(Test-Path "bin") {
    Remove-Item "bin" -Recurse -Force
  }

  if(Test-Path "bin-int") {
    Remove-Item "bin-int" -Recurse -Force
  }
  Remove-Item compile_commands.json -ErrorAction SilentlyContinue
}
else {
  Write-Host "Unknown operation: $operation" -ForegroundColor Red
  Write-Host "build.ps1 (gen|compdb|build|run|clean) [vs2022|vs2026|gmake|DEBUG|STAGING|RELEASE]" -ForegroundColor Cyan
  exit 1
}

$operation = $args[0]
$type = ""

if($operation) { } 
else {
  Write-Host "You need to pass an operation type:" -ForegroundColor Red
  Write-Host "build.ps1 (gen|compdb|build|run|clean) [vs2022|vs2026|gmake|DEBUG|STAGING|RELEASE]" -ForegroundColor Cyan
  exit 1
}

if($operation -eq "gen") {
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
}
else {
  Write-Host "Unknown operation: $operation" -ForegroundColor Red
  Write-Host "build.ps1 (gen|compdb|build|run|clean) [vs2022|vs2026|gmake|DEBUG|STAGING|RELEASE]" -ForegroundColor Cyan
  exit 1
}

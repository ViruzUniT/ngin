$operation = $args[0]
$type = ""

if($operation) { } 
else {
  Write-Host "You need to pass an operation type:" -ForegroundColor Red
  Write-Host "build.ps1 (gen|build|run|clean) [vs2022|vs2026|gmake|DEBUG|STAGING|RELEASE]" -ForegroundColor Cyan
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
      make -C build config=$makeConfig
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
  Write-Host "build.ps1 (gen|build|run|clean) [vs2022|vs2026|gmake|DEBUG|STAGING|RELEASE]" -ForegroundColor Cyan
  exit 1
}

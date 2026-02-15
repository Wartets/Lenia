$ErrorActionPreference = "Stop"
$ProgressPreference = 'SilentlyContinue'
[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12

$libsDir = "libs"
if (!(Test-Path $libsDir)) { New-Item -ItemType Directory -Path $libsDir }

function Download-WithRetry($url, $outFile) {
    Write-Host "Téléchargement de $url..." -ForegroundColor Cyan
    Invoke-WebRequest -Uri $url -OutFile $outFile -TimeoutSec 300
}

# --- 1. GLM ---
if (!(Test-Path "$libsDir/glm")) {
    Download-WithRetry "https://github.com/g-truc/glm/archive/refs/tags/0.9.9.8.zip" "glm.zip"
    Expand-Archive -Path "glm.zip" -DestinationPath "$libsDir" -Force
    Move-Item -Path "$libsDir/glm-0.9.9.8/glm" -Destination "$libsDir/glm"
    Remove-Item -Recurse -Force "glm.zip", "$libsDir/glm-0.9.9.8" -ErrorAction SilentlyContinue
}

# --- 2. GLFW ---
if (!(Test-Path "$libsDir/glfw")) {
    Download-WithRetry "https://github.com/glfw/glfw/releases/download/3.3.8/glfw-3.3.8.bin.WIN64.zip" "glfw.zip"
    Expand-Archive -Path "glfw.zip" -DestinationPath "$libsDir" -Force
    New-Item -ItemType Directory -Path "$libsDir/glfw"
    Move-Item -Path "$libsDir/glfw-3.3.8.bin.WIN64/include" -Destination "$libsDir/glfw/include"
    Move-Item -Path "$libsDir/glfw-3.3.8.bin.WIN64/lib-mingw-w64" -Destination "$libsDir/glfw/lib"
    Remove-Item -Recurse -Force "glfw.zip", "$libsDir/glfw-3.3.8.bin.WIN64" -ErrorAction SilentlyContinue
}

# --- 3. ImGui ---
if (!(Test-Path "$libsDir/imgui")) {
    Download-WithRetry "https://github.com/ocornut/imgui/archive/refs/heads/docking.zip" "imgui.zip"
    Expand-Archive -Path "imgui.zip" -DestinationPath "$libsDir" -Force
    $imguiTemp = Get-ChildItem -Path "$libsDir/imgui-docking*" | Select-Object -First 1
    New-Item -ItemType Directory -Path "$libsDir/imgui"
    New-Item -ItemType Directory -Path "$libsDir/imgui/backends"
    Copy-Item -Path "$($imguiTemp.FullName)/*.cpp", "$($imguiTemp.FullName)/*.h" -Destination "$libsDir/imgui/"
    Copy-Item -Path "$($imguiTemp.FullName)/backends/imgui_impl_glfw.*", "$($imguiTemp.FullName)/backends/imgui_impl_opengl3.*" -Destination "$libsDir/imgui/backends/"
    Remove-Item -Recurse -Force "imgui.zip", $imguiTemp.FullName -ErrorAction SilentlyContinue
}

# --- 4. GLAD (Lien corrigé vers une version pré-générée 4.6 Core) ---
if (!(Test-Path "$libsDir/glad")) {
    Write-Host "Téléchargement de Glad (Version pré-générée)..." -ForegroundColor Cyan
    New-Item -ItemType Directory -Path "$libsDir/glad/src"
    New-Item -ItemType Directory -Path "$libsDir/glad/include/glad"
    New-Item -ItemType Directory -Path "$libsDir/glad/include/KHR"
    
    # Liens vers des versions brutes stables
    Download-WithRetry "https://raw.githubusercontent.com/fbeutel/f_engine/master/lib/glad/src/glad.c" "$libsDir/glad/src/glad.c"
    Download-WithRetry "https://raw.githubusercontent.com/fbeutel/f_engine/master/lib/glad/include/glad/glad.h" "$libsDir/glad/include/glad/glad.h"
    Download-WithRetry "https://raw.githubusercontent.com/fbeutel/f_engine/master/lib/glad/include/KHR/khrplatform.h" "$libsDir/glad/include/KHR/khrplatform.h"
}

Write-Host "`nInstallation terminée ! Lancez maintenant build_windows.bat" -ForegroundColor Green
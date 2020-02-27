set -e

echo "Building..."

GameLib="libWhitherthen.so"
#RendererLib="libWhitherthenSokolGFX.so"

CommonCompilerFlags="-g -ggdb -std=c++11 -msse4.1 -ffast-math -Wno-braced-scalar-init -Wno-format -Wno-writable-strings -Wno-switch -Wno-unused-value"
CommonDefines="-DCOMPILE_INTERNAL=1 -DCOMPILE_SLOW=1 -DCOMPILE_LINUX=1"
CommonLinkerFlags="-pthread -lX11 -ldl -lGL"

curDir=$(pwd)
buildDir="$curDir/../build"
dataDir="$curDir/../data"

pushd $buildDir > /dev/null

#echo Building Renderer
#clang++ -shared -fPIC $CommonCompilerFlags $CommonDefines "$curDir/sokol_renderer.cpp" -o build_$RendererLib -lGL
#rm -f $RendererLib
#mv build_$RendererLib $RendererLib

echo Building Game Library
clang++ -shared -fPIC $CommonCompilerFlags $CommonDefines "$curDir/whitherthen.cpp" -o build_$GameLib

rm -f $GameLib
mv build_$GameLib $GameLib

echo Building Platform
clang++ $CommonCompilerFlags $CommonDefines "$curDir/sokol_whitherthen.cpp" -o whitherthen $CommonLinkerFlags

popd > /dev/null

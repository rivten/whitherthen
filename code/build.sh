CODE_PATH="$(dirname "$0")"

CommonFlags="-g -std=c++11 -Werror -Wall -Wextra -Wcast-align -Wmissing-noreturn -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wmissing-include-dirs -Wno-old-style-cast -Woverloaded-virtual -Wredundant-decls -Wsign-promo -Wstrict-overflow=4 -Wundef -Wno-unused -Wno-variadic-macros -Wno-parentheses -fdiagnostics-show-option -Wno-write-strings -Wno-absolute-value -Wno-cast-align -Wno-unused-parameter -Wno-format-nonliteral -Wno-missing-noreturn -Wno-sign-compare -lm"

if [ -n "$(command -v clang++)" ]
then
	CXX=clang++
	CommonFlags+=" -Wno-missing-braces -Wno-null-dereference -Wno-self-assign"
else
	CXX=c++
	CommonFlags+=" -Wno-unused-but-set-variable"
fi

CommonFlags+=" -DCOMPILE_SLOW=1 -DCOMPILE_INTERNAL=1"

CommonLinkerFlags="-l GL -l X11 -l GLEW -l dl -l pthread"

mkdir -p "$CODE_PATH/../build"
pushd "$CODE_PATH/../build"

$CXX $CommonFlags ../code/sokol_whitherthen.cpp $CommonLinkerFlags -o whitherthen

popd


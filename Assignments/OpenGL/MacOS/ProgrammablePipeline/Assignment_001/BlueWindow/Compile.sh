rm -rf OGLPP.app/Contents/MacOS
mkdir -p OGLPP.app/Contents/MacOS
clang++ -o OGLPP.app/Contents/MacOS/OGLPP OGLPP.mm -framework Cocoa -framework QuartzCore -framework OpenGL -Wno-deprecated-declarations

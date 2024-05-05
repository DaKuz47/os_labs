BUILD_DIR="build"

mkdir $BUILD_DIR
cd $BUILD_DIR
cmake ..
cmake --build .
cd ..
mv ./$BUILD_DIR/host* .
rm -rf $BUILD_DIR

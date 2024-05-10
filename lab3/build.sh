BUILD_DIR="build"
RUN_FILE="run_tests"

mkdir $BUILD_DIR
cd $BUILD_DIR
cmake ..
cmake --build .
cd ..
mv $BUILD_DIR/$RUN_FILE .
rm -rf $BUILD_DIR

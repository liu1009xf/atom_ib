mkdir build
cd build

Build_Config=Release
echo "Building in $Build_Config Model"

cmake ..
cmake --build . --config $Build_Config
ctest -C $Build_Config --verbose --timeout 1200 
read -rsp $'Press any key to continue...\n' -n 1 key
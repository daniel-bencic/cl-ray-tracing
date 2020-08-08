rm -rf build
mkdir build
cd build

if [ "$1" = "-r" ]
then
        mkdir release
        cd release
        cmake ../.. -DCMAKE_BUILD_TYPE=Release
else 
        mkdir debug
        cd debug
        cmake ../.. -DCMAKE_BUILD_TYPE=Debug
fi

make

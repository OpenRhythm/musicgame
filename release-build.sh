#rm -rf build
mkdir build
cd ./build

if [ "$1" = "--noapp" ]
then
	export NO_APP="True"
fi

cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
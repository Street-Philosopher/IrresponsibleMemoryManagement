
# FILE_NAME = "IMM8"

#remove main so it doesn't start if we compile and it has errors
echo deleting the previous build...
rm main

#compile
echo compiling...
clang++ -std=c++17 -LSFML-2.5.1/lib -lsfml-graphics -lsfml-window -lsfml-system src/*.cpp -o main -ISFML-2.5.1/include

echo doing the thing...
#of course i know what these lines do, what do you mean
chmod 744 main
#export LD_LIBRARY_PATH=SFML-2.5.1/lib

#run
echo running...
./main

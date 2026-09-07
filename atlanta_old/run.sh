echo building...
g++ main.cpp -o Atlanta3D -I/opt/sfml2/include -L/opt/sfml2/lib -Wl,-rpath,/opt/sfml2/lib -lsfml-graphics -lsfml-window -lsfml-system
echo done
./Atlanta3D
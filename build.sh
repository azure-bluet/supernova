g++ db/supernova.cpp -o supernova.out -O3 -Idb
g++ -shared -fPIC db/starfruit.cpp -o starfruit.so -O3 -Idb
mkdir data
echo ./starfruit.so > plugins.list
g++ dbcli/daemon.cpp -o daemon.out -O3 -Idbcli
mkdir http/cgi-bin
g++ dbcli/query.cpp -o http/cgi-bin/query -O3 -Idbcli
zip -vr supernova.zip db dbcli http boot.py build.sh license readme data daemon.out plugins.list starfruit.so supernova.out

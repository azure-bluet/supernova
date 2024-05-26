g++ db/supernova.cpp -o supernova.out -O3 -Idb
g++ -shared -fPIC db/starfruit.cpp -o starfruit.so -O3 -Idb
mkdir data
echo ./starfruit.so > plugins.list
mkdir http/cgi-bin
g++ dbcli/query.cpp -o http/cgi-bin/query -O3 -Idbcli
zip -vr supernova.zip db dbcli http boot.py build.sh license readme data plugins.list starfruit.so supernova.out

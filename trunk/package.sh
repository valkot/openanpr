cd Debug
make clean
make
cd ../deb
mkdir usr
cd usr
mkdir bin
cd bin
rm *
cd ../../../Debug
cp openanpr ../deb/usr/bin
cd ..
dpkg -b deb openanpr.deb

#!/bin/sh

echo ""
echo "   ===== building decoder ====="
echo ""
cd decoder
make clean
make

echo ""
echo "   ===== building recorder ====="
echo ""
cd ..
cd recorder
make clean
make

echo ""
echo "   ===== buidling codec ====="
echo ""
cd ..
cd codec
make clean
make
cp -f cdecoder ../recorder/wav/
cp -f sdecoder ../recorder/wav/

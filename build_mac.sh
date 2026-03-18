#!/bin/bash
echo "==> Đóng gói game cho macOS..."
make clean
make

mkdir -p release_mac/SpaceShooter
cp space_shooter release_mac/SpaceShooter/
cp -r assets/ release_mac/SpaceShooter/assets/ 2>/dev/null || true
cp cau_hoi*.csv release_mac/SpaceShooter/ 2>/dev/null || true

cd release_mac
zip -r SpaceShooter_Mac.zip SpaceShooter
cd ..

echo "==> XONG! File tải nằm ở: release_mac/SpaceShooter_Mac.zip"

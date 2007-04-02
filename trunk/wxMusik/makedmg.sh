#!/bin/sh
VERSION="0.4.3"
rm "wxMusik-$VERSION.dmg"
mkdir wxMusik
cp -a wxMusik.app wxMusik 
hdiutil create -verbose -fs HFS+ -srcfolder wxMusik -format UDZO -imagekey zlib-level=9 -volname "wxMusik-$VERSION" "wxMusik-$VERSION"
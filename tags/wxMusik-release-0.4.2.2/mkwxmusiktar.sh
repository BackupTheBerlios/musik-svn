cd ..
VERSION="0.4.2.2"
mv wxMusik "wxMusik-$VERSION"
tar -cf "wxMusik-$VERSION.tar" --exclude=.svn  \
	--exclude=*.o --exclude=wxMusik --exclude=*.psd \
	--exclude=*.a --exclude=*~ --exclude=.* \
	--exclude=crelbuild --exclude=cdebbuild  "wxMusik-$VERSION"
#tar --append -f "wxMusik-$VERSION.tar" "wxMusik-$VERSION/CMakeLists.txt"
bzip2 -f "wxMusik-$VERSION.tar"
mv "wxMusik-$VERSION" wxMusik



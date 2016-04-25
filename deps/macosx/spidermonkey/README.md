Build
=====


Clone the mozilla-central repository.

```
DEST_PATH = ~/Development/delphinus/deps/macosx/spidermonkey

cd mozilla-central/js/src
mkdir build_OPT.OBJ
cd build_OPT.OBJ

# --disable-shared-js

# For debug build
../configure --disable-tests --enable-llvm-hacks --without-intl-api --enable-debug

# For release build
../configure --disable-tests --enable-release --disable-debug --enable-strip --enable-install-strip --enable-optimize=-O3 --enable-llvm-hacks --without-intl-api

xcrun make -j4

rm -R $DEST_PATH/include/
cp -RL dist/include/ $DEST_PATH/include/

cp -RL dist/bin/js $DEST_PATH
cp -RL dist/bin/libmozglue.dylib $DEST_PATH
cp -RL dist/sdk/lib/libmozglue.dylib $DEST_PATH
cp -RL dist/sdk/lib/libmozjs-45.dylib $DEST_PATH
cp js/src/libjs_static.a $DEST_PATH

cd $DEST_PATH

# Release only
xcrun strip -S libjs_static.a
xcrun lipo -info libjs_static.a

```

#!/bin/bash
# bundle_macdist.sh - create a mac release of a qt program
# written by Christian Vogelgsang <chris@vogelgsang.org>
# under the GNU Public License V2

APP=DiskImagery64
VERSION_FILE=imagery/version.h
VERSION=`awk '{ print $3 }' $VERSION_FILE | sed -e 's/"//g'`
BUILD_DIR="BUILD/$APP-$VERSION"

echo "--- bundle_macdist ---"

if [ "$QTDIR" = "" ]; then
	QTDIR=/usr/local/Trolltech/Qt-4.2.2
fi
if [ ! -d "$QTDIR" ]; then
	echo "FATAL:No Qt install found at '$QTDIR'!"
	exit 1
fi
echo "qt: $QTDIR"

# clean all
clean() {
  if [ -e Makefile ]; then
	  echo "make distclean"
	  make distclean
  fi
  if [ -d BUILD ]; then
  	rm -rf BUILD
  fi
}

# build
build() {
  # call qmake
  $QTDIR/bin/qmake "CONFIG+=release"
  # build 
  make
  # check app
  if [ ! -d BUILD/$APP.app ]; then
	  echo "Build failed!"
		exit 1
  fi
}

# bundle
bundle() {
	echo "-- Bundling Version: $VERSION"
	
	# setup dist dir
	if [ -d "$BUILD_DIR" ]; then
		rm -rf "$BUILD_DIR"
	fi
	mkdir -p "$BUILD_DIR"
	
	# install app
	cp -r BUILD/$APP.app "$BUILD_DIR/"
	
	# embed qt frameworks
	QT_VERSION="4"
	FRAMEWORK_DIR="$BUILD_DIR/$APP.app/Contents/Frameworks"
	mkdir -p "$FRAMEWORK_DIR"
	EXE="$BUILD_DIR/$APP.app/Contents/MacOS/$APP"
	otool -L "$EXE" | grep "$QTDIR" | awk '{ print $1 }' > fw
	for fw in `cat fw` ; do
		FRAMEWORK="`echo \"$fw\" | sed -e 's,^.*/\(.*.framework\).*$,\1,'`"
		echo " embedding $FRAMEWORK"
		# copy framework
		(cd "$QTDIR/lib" && tar cf - --exclude Headers --exclude '*_debug' "$FRAMEWORK" ) | (cd "$FRAMEWORK_DIR" && tar xf -)
		# fix reference
		FRAMEWORK_REF="`echo \"$fw\" | sed -e \"s,$QTDIR/lib,@executable_path/../Frameworks,\"`"
		echo " fixing ref to framework $FRAMEWORK_REF"
		install_name_tool -change "$fw" "$FRAMEWORK_REF" "$EXE"
		# check framework itself
		BASENAME="`echo \"$FRAMEWORK\" | sed -e 's,.framework,,g'`"
		LIBPATH="$FRAMEWORK_DIR/$FRAMEWORK/Versions/$QT_VERSION/$BASENAME"
		echo " fixing framework $LIBPATH"
		if [ ! -e "$LIBPATH" ]; then
			echo "ERROR: missing: $LIBPATH"
			exit 1
		fi
		# fix lib name
		LIBID="@executable_path/../Frameworks/$FRAMEWORK/Versions/$QT_VERSION/$BASENAME"
		echo "   install name: $LIBID"
		install_name_tool -id "$LIBID" "$LIBPATH"
		# check lib
		otool -L "$LIBPATH" | grep "$QTDIR" | awk '{ print $1 }' > fw2
		for fw2 in `cat fw` ; do
			FRAMEWORK_REF2="`echo \"$fw2\" | sed -e \"s,$QTDIR/lib,@executable_path/../Frameworks,\"`"
			echo "   fixing ref to framework $FRAMEWORK_REF2"
			install_name_tool -change "$fw2" "$FRAMEWORK_REF2" "$LIBPATH"
		done
		rm -f fw2
	done
	rm -f fw
	
	# copy add ons
  cp fonts/CBM.dfont "$BUILD_DIR/"
  cp fonts/CBMShift.dfont "$BUILD_DIR/"
  cp README.txt "$BUILD_DIR/"
}

# create disk image
make_dmg() {
	echo "-- Creating Disk Image"
	# image name
  BUILD_IMG="BUILD/$APP-$VERSION-mac.dmg"
  BUILD_TMP_IMG="BUILD/$APP-$VERSION-mac.tmp.dmg"
  BUILD_VOL="$APP-$VERSION"
	if [ ! -d "$BUILD_DIR" ]; then
		echo "ERROR: $BUILD_DIR does not exist!"
		exit 1
	fi

  # Create the image and format it
  echo "  creating DMG"
  hdiutil create -srcfolder $BUILD_DIR $BUILD_TMP_IMG -volname $BUILD_VOL -ov -quiet 

  # Compress the image
  echo "  compressing DMG"
  hdiutil convert $BUILD_TMP_IMG -format UDZO -o $BUILD_IMG -ov -quiet
  rm -f $BUILD_TMP_IMG

  echo "ready. created dist file: $BUILD_IMG"
	if [ ! -e "$BUILD_IMG" ]; then
		echo "ERROR: $BUILD_IMG not created!"
		exit 1
	fi
  du -sh $BUILD_IMG
  md5 -q $BUILD_IMG
}

clean
build
bundle
make_dmg

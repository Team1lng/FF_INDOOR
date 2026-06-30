all:
	rm -f build/CMakeFiles/FF.BIN.dir$(CURDIR)/layout/layout_logo.c.o
	cd build ; cmake .;make -j16
	rm -rf upgrade/app
	mkdir -p upgrade/app
	cp -f build/FF.BIN 	upgrade/app
	cp -f res/rom.bin 		upgrade/app
	cp -f res/sat_leo.ttf 	upgrade/app
	cp -f res/persian.ttf 	upgrade/app
	cp -rf res/rings 		upgrade/app
	cd upgrade/;./make_image.sh

clean:
	mv ./build/CMakeLists.txt ./
	rm build/* -rf
	mv CMakeLists.txt ./build/

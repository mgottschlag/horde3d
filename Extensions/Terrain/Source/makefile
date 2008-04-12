# Terrain Extension

OBJECTS = \
	extension.o terrain.o

libHorde3DTerrain: $(OBJECTS)
	g++ $(OBJECTS) -olibHorde3DTerrain.so -shared
	mv libHorde3DTerrain.so ../../../Horde3D/Binaries/Linux_x86/

%.o: %.cpp
	g++ -c $< -o$@ -I../../../Horde3D/Source/Shared -I../../../Horde3D/Source/Horde3D\ Engine -fPIC ${CFLAGS}


clean:
	rm *.o; rm ../../../Horde3D/Binaries/Linux_x86/libHorde3DTerrain.so

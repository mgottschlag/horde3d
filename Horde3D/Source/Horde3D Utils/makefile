# Horde3D Utils

OBJECTS = main.o


libHorde3D: $(OBJECTS)
	g++ $(OBJECTS) -olibHorde3DUtils.so -L../../Binaries/Linux_x86 -lHorde3D -shared
	mv libHorde3DUtils.so ../../Binaries/Linux_x86

%.o: %.cpp
	g++ -c $< -o$@ -I../../Bindings/C++ -I../Shared -fPIC ${CFLAGS}


clean:
	rm *.o; rm ../../Binaries/Linux_x86/libHorde3DUtils.so
# Sample Terrain

OBJECTS = app.o main.o


libHorde3D: $(OBJECTS)
	g++ $(OBJECTS) -oSample_Terrain -L../../../Horde3D/Dependencies/Libs_Linux_x86 -L../../../Horde3D/Binaries/Linux_x86 -lglfw -lXrandr -lpthread -lHorde3D -lHorde3DUtils -lHorde3DTerrain -Wl,-rpath,'$$ORIGIN'
	mv Sample_Terrain ../../../Horde3D/Binaries/Linux_x86

%.o: %.cpp
	g++ -c $< -o$@ -I../Bindings/C++ -I../../../Horde3D/Bindings/C++ -I../../../Horde3D/Dependencies/Include ${CFLAGS}


clean:
	rm *.o; rm ../../../Horde3D/Binaries/Linux_x86/Sample_Terrain

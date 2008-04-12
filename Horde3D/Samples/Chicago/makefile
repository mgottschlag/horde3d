# Sample Chicago

OBJECTS = app.o crowd.o main.o


libHorde3D: $(OBJECTS)
	g++ $(OBJECTS) -oSample_Chicago -L../../Dependencies/Libs_Linux_x86 -L../../Binaries/Linux_x86 -lglfw -lXrandr -lpthread -lHorde3D -lHorde3DUtils -Wl,-rpath,'$$ORIGIN'
	mv Sample_Chicago ../../Binaries/Linux_x86

%.o: %.cpp
	g++ -c $< -o$@ -I../../Bindings/C++ -I../../Dependencies/Include ${CFLAGS}


clean:
	rm *.o; rm ../../Binaries/Linux_x86/Sample_Chicago
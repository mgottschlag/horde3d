# Collada Converter

OBJECTS = converter.o daeMain.o main.o optimizer.o utils.o ../Shared/utXMLParser.o


libHorde3D: $(OBJECTS)
	g++ $(OBJECTS) -oColladaConv -L../../Dependencies/Libs_Linux_x86
	mv ColladaConv ../../Binaries/Linux_x86

%.o: %.cpp
	g++ -c $< -o$@ -I../../Dependencies/Include -I../Shared ${CFLAGS}


clean:
	rm *.o; rm ../../Binaries/Linux_x86/ColladaConv
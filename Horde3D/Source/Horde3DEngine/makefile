# Horde3D Engine

OBJECTS = \
	egAnimatables.o egAnimation.o egCamera.o egCom.o egExtensions.o egGeometry.o egLight.o egMain.o \
	egMaterial.o egModel.o egModules.o egParticle.o egPipeline.o egPrimitives.o egRendererBase.o \
	egRenderer.o egResource.o egScene.o egSceneGraphRes.o egShader.o egTextures.o \
	utImage.o utOpenGL.o ../Shared/utXMLParser.o


libHorde3D: $(OBJECTS)
	g++ $(OBJECTS) -olibHorde3D.so -lGL -shared
	mv libHorde3D.so ../../Binaries/Linux_x86

%.o: %.cpp
	g++ -c $< -o$@ -I../Shared -I../../Extensions -fPIC ${CFLAGS}


clean:
	rm *.o; rm ../../Binaries/Linux_x86/libHorde3D.so

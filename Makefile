CPPFLAGS = -I/opt/vc/include
CFLAGS = -g -Wall -Werror
LDFLAGS = -g -L/opt/vc/lib
LDLIBS = -lbcm_host -lbrcmEGL -lbrcmGLESv2 -lftdi -lm

cube-shader: cube-shader.o bcm.o egl.o glprog.o leds.o mpsse.o

clean:
	rm -f *.o cube-shader

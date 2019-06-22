CPPFLAGS = -I/opt/vc/include
CFLAGS = -g -Wall -Werror
LDFLAGS = -g -L/opt/vc/lib
LDLIBS = -lbcm_host -lbrcmEGL -lbrcmGLESv2 -lftdi -lm

OFILES := cube-shader.o bcm.o egl.o glprog.o leds.o mpsse.o noise.o	\
          preproc.o str-array.o strbuf.o

cube-shader: $(OFILES)

clean:
	rm -f *.o cube-shader

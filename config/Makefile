SRCS = new_conf.c

OBJS = out/new_conf

# for arm
# LIBS += -lvpu -lipu -lrt
#LIBS += -L./
INC += -I./

CFLAGS 	+= -Wall -Wstrict-prototypes -Wno-trigraphs -O2 \
	  	   -fno-strict-aliasing -fno-common -lpthread -L./lib -lconfig

$(OBJS): $(SRCS)
	mkdir -p ./out
	$(VERBOSE) $(CC) -g $(INC) $(CFLAGS) $^ $(LIBS) -o $@

# arm-linux-gnueabihf-g++ -g $(INC) $^ $(LIBS) -o $@
# $(VERBOSE) $(CC) -g $(INC) $(CFLAGS) $^ $(LIBS) -o $@

all: $(OBJS)

.PHONY: clean
clean:
	rm -rf out

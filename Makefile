CC = arm-none-eabi-gcc
LD = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
RM = rm

#extra warnings for windows
CFLAGS := -DVFP -nostartfiles -O2 -mfloat-abi=hard -mfpu=vfp -march=armv6zk -mtune=arm1176jzf-s -fno-delete-null-pointer-checks -fdata-sections -ffunction-sections --specs=nano.specs --specs=nosys.specs -u _printf_float
#CFLAGS := -DVIDULA -nostartfiles -O2 -mcpu=arm7tdmi -mtune=arm7tdmi -fno-delete-null-pointer-checks -fdata-sections -ffunction-sections --specs=nano.specs --specs=nosys.specs -u _printf_float
LD_FLAGS := -Wl,--gc-sections -Wl,--no-print-gc-sections -Wl,-T,rpi.X -Wl,-lm

CFLAGS += -Dbagi_SDLENV

OBJ = main.o armc-start.o armtubeio.o armtubeswis.o beebScreen/beebScreen.o sqrtf.o

LIB = -lc -lm

rt: $(OBJ)
	$(CC) $(CFLAGS) $(LD_FLAGS) $(OBJ) $(LIB) -o temp
	$(OBJCOPY) -O binary temp $@

cleanall: clean depclean 

clean: 
	$(RM) $(OBJ) rt beebScreen/*.bin

remake: clean rt

beebScreen/beebScreen.o: beebScreen/beebScreen.c beebScreen/beebCode.c beebScreen/beebScreen.h
	$(CC) $(CFLAGS) -c beebScreen/beebScreen.c -o beebScreen/beebScreen.o

beebScreen/beebCode.c: beebScreen/beebCode.bin
	cd beebScreen; ./mkasm.bat

beebScreen/bagiCode.c: beebScreen/beebCode.asm beebScreen/bagiCode.bin
	cd beebScreen; ./mkasm.bat

beebScreen/beebCode.bin: beebScreen/beebCode.asm

beebScreen/bagiCode.bin: beebScreen/beebCode.asm

main.c: beebScreen/beebCode.c beebScreen/bagiCode.c

depclean:
	$(RM) $(OBJ:.o=.d)

%.d: %.c
	$(CC) -M -MG $(DEPFLAGS) $< | sed -e 's@ /[^ ]*@@g' -e 's@^\(.*\)\.o:@\1.d \1.o:@' > $@
#sh depend.sh $(DEPFLAGS) $< > $@

include $(SRC:.c=.d)
	
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(CC) $(CFLAGS) -c $< -o $@
	


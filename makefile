CC = gcc


TARGET = translate


OBJS = main.o address.o stat.o

$(TARGET): $(OBJS)
		$(CC) -o $(TARGET) $(OBJS)

main.o: main.c lab4.h
		$(CC) -c main.c 

address.o: address.c lab4.h
		$(CC) -c address.c 

stat.o: stat.c lab4.h
		$(CC) -c stat.c

clean:
		rm -f $(OBJS) $(TARGET)
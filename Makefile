all:
	gcc linux.c -o st_mal -pthread -lcurl
clean:
	rm -f st_mal
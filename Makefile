all:
	gcc linux.c -o ss_mal -pthread -lcurl
clean:
	rm -f ss_mal
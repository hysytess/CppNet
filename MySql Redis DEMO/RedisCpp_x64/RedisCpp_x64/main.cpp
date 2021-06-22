#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>

#include "./deps/hiredis/hiredis.h"

int main(int argc, char* args[])
{

	redisContext *_connect;
	redisReply *_reply;
	struct timeval timeout = { 1, 500000 }; 
	_connect = redisConnectWithTimeout("127.0.0.1", 6379, timeout);
	if (_connect == NULL || _connect->err) {
		if (_connect) {
			printf("Connection error: %s\n", _connect->errstr);
			redisFree(_connect);
		}
		else {
			printf("Connection error: can't allocate redis context\n");
		}
		//exit(1);
	}

	/* Set a key */
	_reply = (redisReply *)redisCommand(_connect, "auth %s", "redispassword");
	printf("AUTH %s\n", _reply->str);
	freeReplyObject(_reply);

	/* PING server */
	_reply = (redisReply*)redisCommand(_connect, "PING");
	printf("PING: %s\n", _reply->str);
	freeReplyObject(_reply);

	/* Set a key */
	_reply = (redisReply *)redisCommand(_connect, "SET %s %s", "2623999858", "mimapassword");
	printf("SET: %s\n", _reply->str);
	freeReplyObject(_reply);
	/* set expire a key */
	_reply = (redisReply *)redisCommand(_connect, "expire %s %s", "2623999858", "10");
	printf("%s\n", _reply->str);
	freeReplyObject(_reply);

	for (size_t i = 0; i < 15; i++)
	{
		/* Try a GET and two INCR */
		_reply = (redisReply *)redisCommand(_connect, "GET 2623999858");
		printf("GET 2623999858: %s\n", _reply->str);

		if (i == 5)
		{
			/* Set a key */
			_reply = (redisReply *)redisCommand(_connect, "SET %s %s", "2623999858", "password");
			freeReplyObject(_reply);
			_reply = (redisReply *)redisCommand(_connect, "expire %s %s", "2623999858", "5");
			freeReplyObject(_reply);
		}
		if ( i == 8)
		{
			_reply = (redisReply *)redisCommand(_connect, "del %s", "2623999858");
			freeReplyObject(_reply);
		}
		Sleep(1000);
	}

	system("pause");
	return 0;
}
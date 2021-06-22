#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "mysql.h"

#pragma comment(lib,"libmysql")

int main(int argc, char* args[])
{
	MYSQL dbobj;
	mysql_init(&dbobj);

	MYSQL_RES *results;
	MYSQL_ROW record;


	if (!mysql_real_connect(&dbobj,"127.0.0.1","DevUser","password","useraccountdb",3306,NULL,0))
	{
		printf("Connet to MySQL Server fail.\n");
		mysql_close(&dbobj);
		return -1;
	}
	mysql_query(&dbobj, "select * from accountlist;");

	results = mysql_store_result(&dbobj);

	while ((record = mysql_fetch_row(results))) {
		printf("%s - %s \n", record[0], record[1]);
	}

	mysql_free_result(results);
	mysql_close(&dbobj);
	mysql_server_end();

	system("pause");
	return 0;
}
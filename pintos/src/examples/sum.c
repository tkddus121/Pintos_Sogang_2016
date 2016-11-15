#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"syscall.h"
#include"../lib/user/syscall.h"


int 
main(int argc, char* argv[])
{
	int a,b,c,d;

	int sum = 0,fibo = 0;

	/*
	scanf("%d %d %d %d",&a,&b,&c,&d);
	sum = sum_of_four_integers(a,b,c,d);
	*/

//	printf("sum ");
	a = atoi(argv[1]); b = atoi(argv[2]);
	c = atoi(argv[3]); d = atoi(argv[4]);


	fibo = fibonacci(a);
	sum = sum_of_four_integers(a,b,c,d);

	printf("%d %d\n",fibo,sum);

	return EXIT_SUCCESS;
}



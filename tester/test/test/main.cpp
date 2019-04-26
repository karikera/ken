#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<time.h>
#define ITERATION 100000  //반복 횟수
#define seven 7
#define SEED 100
int one = 0, two = 0, tri = 0, full = 0, four = 0, flush = 0, stra = 0, stf = 0;
float Uniform()
{
	float rv;
	rv = rand() / (float)RAND_MAX;
	return rv;
}
int number()
{
	int num = 0;
	num = rand() % 13;
	return num;
}
int type()
{
	int rv;
	rv = rand() % 4;
	return rv;
}
void sort(int(*a)[2])
{
	int temp[1][2];
	int i, j, k;
	for (i = 0; i < 6; i++)
	{
		for (j = i + 1; j < 7; j++)
		{
			if (a[i][0]>a[j][0])
			{
				temp[0][0] = a[i][0];
				temp[0][1] = a[i][1];
				a[i][0] = a[j][0];
				a[i][1] = a[j][1];
				a[j][0] = temp[0][0];
				a[j][1] = temp[0][1];
			}
		}
	}
	jok(a);
}
int jok(int(*a)[2])				//아직덜짬
{
	int cnt = 0, check = 0;
	int what[14] = { 0 };
	int i;
	int one1 = 0, tri1 = 0, four1 = 0, stra1 = 0;

	for (i = 0; i < 7; i++)
	{
		check = a[i][0];
		what[check]++;
	}
	for (i = 0; i < 14; i++)
	{
		if (what[i] == 2)
			one1++;
		else if (what[i] == 3)
			tri1++;
		else
			four1++;
	}
	//	if (one1 == 2)
}
/*
for (i = 0; i < 6; i++)
{
if ((a[i][0] + 1) == a[i + 1][0])
cnt++;
else
cnt = 0;
}
if (cnt == 5)
stra1++;
for (i = 0; i < 5; i++)					//원페 투페
{
if (a[i][0] == a[i + 1][0] && a[i+1][0]!=a[i+2][0])
{
one1++;
}
}
for (i = 0; i < 4; i++)					//트리플
{
if (a[i][0] == a[i + 1][0] && a[i + 1][0] == a[i + 2][0] && a[i+2][0]!=a[i+3][0])
tri1++;
}
for (i = 0; i < 3; i++)					//포카
{
if (a[i][0] == a[i + 1][0] && a[i + 1][0] == a[i + 2][0] && a[i + 2][0] == a[i + 3][0] && a[i + 3][0] != a[i + 4][0])
four1++;
}
if (one1 == 1 && tri1==0 && four1==0 && stra1==0)
one++;
else if (one1 == 2 || one1==3 && tri1 == 0 && four1 == 0 && stra1 == 0)
two++;
else if (tri1 == 1 && one1 == 0 && four1 == 0 && stra1 == 0)
tri++;
else if (stra1 == 1 && four == 0)
stra++;
else if (four1 == 1)
four++;
*/

int duple(int(*a)[2], int(*b)[1], int i)
{
	int j;
	for (j = 0; j < i; j++)
	{
		if (a[j][0] == b[0][0] && a[j][1] == b[0][1])
			return 1;
		else
			return 0;
	}
}
void sevenpick()
{
	int i, j, k;
	int a[7][2];
	int b[1][2];
	for (i = 0; i < 7; i++)					//a 초기화
		for (j = 0; j < 2; j++)
			a[i][j] = 0;
	for (i = 0; i < 7; i++)
	{
		j = 0;
		a[i][j] = number();
		b[0][j] = a[i][j];
		j++;
		a[i][j] = type();
		b[0][j] = a[i][j];
		if (1 == duple(&a, &b, i))
			i--;
		printf("%d ", a[i][0]);
	}
	printf("\n");
	sort(&a);
	jok(&a);
}
void main()
{
	int i, j, k;

	int a[7][2];		//앞에께 숫자 뒤에께 문양
	srand(time(NULL));
	//srand(SEED);
	for (i = 0; i < 1; i++)
		sevenpick();
	printf("%d %d %d %d %d ", one, two, tri, four, stra);
}
float add(float x, int y)
{
	return x + y;
}


int main()
{
    int a=11;
    int b=22;
    a = a+b;
	int c[3] = {1, 2, 3};
	float d = 3.14;
	float e = a * d;
	float f = add(e, c[1]);
    return 0;
}


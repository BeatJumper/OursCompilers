int to[5005], next[5005], head[1005], cnt = 0;

void add_edge(int from, int To){
	to[cnt] = To;
	next[cnt] = head[from];
	head[from] = cnt;
	cnt = cnt + 1;
	to[cnt] = from;
	next[cnt] = head[To];
	head[To] = cnt;
	cnt = cnt + 1;
}

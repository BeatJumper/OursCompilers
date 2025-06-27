int func1(int x) {
    if (x > 0) {
        return x + 1;
    } else {
        return x - 1;
    }
}

int func2(int y) {
    if (y > 5) {
        if (y > 10) {
            return y * 2;
        } else {
            return y + 5;
        }
    } else {
        return y - 1;
    }
}

int main() {
    int a = 5;
    int b = func1(a);
    int c = func2(b);
    return c;
}

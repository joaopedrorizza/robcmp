template <typename T>
T square(T value)
{
    return value * value;
}

int main()
{

int entries[] = {
        1, 10, -4, 8, 100, 21, 3, -9, 15, 7,
        -5, 22, 11, -13, 0, 3, 99, -100, 33, 50,
        -10, 125, -128, 64, 32, -32, 12, -90, 90, 5,
        7, -7, 9, 9, -50, 127, -126, 40, -40, 60,
        30, -30, 10, -10, 37, -37, 48, -48, 99, -99,
        2, 5, -4, 10, 50, 22, -1, 0, 14, 9,
        -9, 21, 7, -10, -3, 30, 100, -50, 32, 10,
        -11, 120, -128, 63, 31, -33, 13, -89, 88, 6,
        7, -8, 10, 5, -51, 120, -127, 41, -39, 61,
        29, -31, 8, -11, 38, -36, 45, -47, 101, -98 };
int i =0;

    while (i < 100)

    {
        int expected = entries[i] * entries[i];
        int squaredValue = square(entries[i]);
        if (squaredValue != expected)
        {
            return 1;
        }
        i = i + 1;
    }

    return 0;
}
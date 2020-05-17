#define OK "O%c\n", 'K'
#define abs(a) (( (a) > 0) ? (a) : -(a))
#define line_number(a) printf("%s line: %d\n", a, __LINE__)
#define max(a, b) ((a > b) ? (a) : (b))
printf("abs(4): %d\n", abs(4));
printf("abs(-4): %d\n", abs(-4));
printf("max(1, 2): %d\n", max(1, 2));
printf("max(2, 1): %d\n", max(2, 1));
printf(OK);
line_number("this line 2?");

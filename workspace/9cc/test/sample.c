int main () {
  printf("%d, %s\n",
#if 0
            1,
#elif 0
            2,
#elif 3+5
            3,
#elif 1*5
            4,
#endif
            "3");
    return 0;
}

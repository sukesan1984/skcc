
int main() {
  int x = 0;
  switch (3) {
      case 1:
          x = 1;
          break;
      default:
          x= 4;
          break;
      case 2:
          x = 2;
          break;
  }
  printf("%d\n", x);
  return 0;
}

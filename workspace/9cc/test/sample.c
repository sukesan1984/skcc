
int main() {
    printf("  sub rax, %d   # raxを%sのoffset:%d分だけ押し下げたアドレスが%sの変数のアドレス。それをraxに保存)\n", 10, "hoge", 10, "hoge");  // raxをoffset文だけ押し下げ（nameの変数のアドレスをraxに保存)
    //printf("  %d %sのoffset:%d%s\n", 10, "hoge", 10, "hoge");  // raxをoffset文だけ押し下げ（nameの変数のアドレスをraxに保存)
    //printf("  %d %sのoffset:%d%s\n", 10, "hoge", 10, "hoge");  // raxをoffset文だけ押し下げ（nameの変数のアドレスをraxに保存)
    return 0;
}

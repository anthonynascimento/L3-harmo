#include <stdio.h>

int somme(int a, int b) {
    int aj = 3;
    int i;
    if(aj==3) {
      aj++;
    }
    for(i=0 ; i<10;i++) {
      aj+=1;
    }
    int j=0;
    while(j<5) {
      aj+=1;
      j++;
    }
    return a + b;
}

int division(int a, int b) {
    return a / b;
}

void hello_world() {
    printf("Hello World!\n");
}

int main(int argc, char *argv[]) {
    int a=1, b=2;
    hello_world();
    printf("%d \n",somme(a, b));
    printf("%d \n",division(a, b));
    return 0;
};

#include <iostream>
#include <stdio.h>
#include <string.h>

using namespace std;


struct ChannelSet
{
    char *name;
};

bool read(ChannelSet set[],int length)
{
    // printf("\nHello World! \n");
    for (int i = 0;i < length; ++i)
    {
        char temp[] = "nice";
        set[i].name = temp;
    }
    return true;
}


int main(int argc, char const *argv[])
{

    // printf("\nHello World! \n");
    int length = 10;
    ChannelSet set[length];
    for (int i = 0;i < length; ++i)
    {
        char temp[] = "test";
        set[i].name = temp;
    }
    // char *test2 = "one";
    // char *test3;
    // test3 = strcat(test1,test2);
    // printf("%s\n", test3);
    char temp1[] = "Stand ";
    char temp2[] = "as one!";


    // string test1 = "All ";
    // string test2 = "for one!";
    char *test3 = strcat(temp1,temp2);
    printf("%s\n", test3);
    for (int i = 0;i < length; ++i)
    {
        printf("-- %s\n", set[i].name);
    }

    read(set,length);

    printf("## %i\n", (sizeof(set)/sizeof(*set)) );
    for (int i = 0;i < length; ++i)
    {
        printf("&& %s\n", set[i].name);
    }

    return 0;

}
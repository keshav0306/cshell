#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char *tokenize(char *main_str, char *delimit, char **storage)
{
    int breakpoint = 0;
    if (*storage != NULL && storage[0][0] == 0)
    {
        return NULL;
    }
    if (main_str == NULL)
    {
        main_str = malloc(100);
        strcpy(main_str, *storage);
    }
    *storage = malloc(100);
    int len = strlen(main_str);
    int i;
    char *retval = NULL;
    for (i = 0; i < len; i++)
    {
        char c = main_str[i];
        for (int j = 0; j < strlen(delimit); j++)
        {
            if (main_str[i] == delimit[j])
            {
                breakpoint = 1;
                retval = malloc(sizeof(100));
                strncpy(retval, main_str, i);
                break;
            }
        }
        if (retval != NULL)
        {
            break;
        }
    }
    if (breakpoint == 0)
    {
        retval = malloc(sizeof(100));
        strncpy(retval, main_str, i);
        storage[0][0] = 0;
        if (strlen(retval) == 0)
        {
            return NULL;
        }
        return retval;
    }
    int found_valid_str = 0;
    for (int j = i + 1; j < len; j++)
    {
        int next_is_valid = 1;
        for (int k = 0; k < strlen(delimit); k++)
        {
            if (main_str[j] == delimit[k])
            {
                next_is_valid = 0;
                break;
            }
        }
        if (next_is_valid == 1)
        {
            strcpy(*storage, &main_str[j]);
            found_valid_str = 1;
            break;
        }
    }
    if (found_valid_str == 0)
    {
        storage[0][0] = 0;
    }
    if (strlen(retval) == 0)
    {
        return NULL;
    }
    if(strlen(*storage) == 0){
        *storage = NULL;
    }
    return retval;
}

// int main()
// {
//     char s[10000];
//     scanf("%s", s);
//     char *str;
//     char *stor;
//     for (str = s;; str = NULL)
//     {
//         char *token = tokenize(str, ";:", &stor);
//         if (token == NULL)
//         {
//             break;
//         }
//         printf("%s\n", token);
//     }
// }
#include <stdlib.h>
#include <stdio.h>
#include "ulib_evu_v7.h"
#include <string.h>
#define DEVELOPER_NAME1 "Georgios Stylianakis-akatallilos"
/*explaining anepignwstos:
 * DEVELOPER_NAME2 is characterized by that adjective following his behavioral attributes.
 * Why is that? Let me explain...
 * The following person has danger-ignorance about his ideas and proposals on stuff
 * related to academic/laboratory projects. Always there to argue about his ideas
 * on why there are good to be implemented this way, and that "it won't be that hard,
 * it will be cool"... Following by utter disaster and miserable deadlines.
 * Author: DEVELOPER_NAME1
 * for more informations please feel free to contact me in geostyl@ics.forth.gr
*/
#define DEVELOPER_NAME2 "Evangelos Maliaroudakis-anepignwstos"

void copy_name(char* src, char* name, int* indx)
{
    int32_t name_size = strlen(name);
    for(int i = 0; i < name_size; ++i){
        src[(*indx)++] = name[i];
    }
}

char* construct_team_members(void)
{
    int32_t members_size = strlen(DEVELOPER_NAME1) + strlen(DEVELOPER_NAME2) + 2;
    char* team_members = (char*)calloc(1,members_size*sizeof(char));
    int32_t i = 0;
    /*Georgis Stylianakis-akatallilos Evangelos Maliaroudakis-anepignwstos*/
    copy_name(team_members,DEVELOPER_NAME1, &i);
    team_members[i++] = ' ';
    copy_name(team_members, DEVELOPER_NAME2, &i);
    return team_members;
}

char* reverse_names(char* str)
{
    int32_t str_size = strlen(str);
    char temp;
     for (int32_t i = 0; i < str_size/2; ++i)
    {
        // temp variable use to temporary hold the string
        temp = str[i];
        str[i] = str[str_size - i - 1];
        str[str_size - i - 1] = temp;
    }

    return str;
}

void dma_test(){
    /*Wanna open the device*/
    eduv7_open();
    /*Wanna write*/
    char* team_members = construct_team_members();
    printf("Lets write our names - %s\n", team_members);
    int32_t members_size = strlen(DEVELOPER_NAME1) + strlen(DEVELOPER_NAME2) + 2;
    eduv7_write(team_members, members_size);

    /*lets read something*/
    printf("lets read our names back");
    char* retrieve_team_members = calloc(1,sizeof(members_size));
    eduv7_read(retrieve_team_members, members_size);
    printf("Our names are :\n%s\n", retrieve_team_members);

    /*reverse the names write the again and read them back*/
    char* reversed_team_members = reverse_names(team_members);
    eduv7_write(reversed_team_members, members_size);
    eduv7_read(retrieve_team_members, members_size);
    printf("Our names in reverse order are :\n%s\n", retrieve_team_members);

    /*Wanna close the device*/
    eduv7_close();
}

int main(){
    int* retrieved = (int*) malloc(sizeof(int));
    int* data = (int*) malloc(sizeof(int));
    *data = 0x5ca1ab1e;
    eduv7_open();
    eduv7_write(data,sizeof(int));
    eduv7_read(retrieved,sizeof(int));
    printf("Retrieved 0x%08x\n",~*(int*)retrieved);
    eduv7_close();


    return 0;
}

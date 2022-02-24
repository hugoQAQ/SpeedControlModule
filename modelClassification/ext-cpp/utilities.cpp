#include"utilities.hpp"

int const_getFileNum()
{
    FILE* crs = popen(GET_FILE_NUM_CMD, "r"); // execute the shell command 

    char result[4] = "0";

    fread(result, sizeof(char), sizeof(result), crs);

    if (NULL != crs)
    {
        fclose(crs);
        crs = NULL;
    }

    int num = atoi(result);
    return num;
}


int deepLearn(int i){

	char array0[100]="";
	sprintf(array0, "%d.png", i);
	
	// char cmd_img[] = "lsix Signs/";
	// strcat(cmd_img, array0);
	// int img = system(cmd_img);
	
	char cmd_limit[] = "./ext-cpp/limit ext-cpp/model.pt Signs/";
	strcat(cmd_limit, array0);
	
	char file []= " > system.txt";
	strcat(cmd_limit, file);

	int a = system(cmd_limit);
	   FILE *fp;
	   int buff;

	   fp = fopen("system.txt", "r");
	   fscanf(fp, "%d", &buff);
	   fclose(fp);
	// char cmd_proba[] = "./proba model.pt ../Signs/";
	// strcat(cmd_proba, array0);
 	// int b = system(cmd_proba);

	return buff;

}





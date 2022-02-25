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
	char buffer [50];
	sprintf (buffer, "%d.png", i);
	stringstream tmp;
	//tmp<<"python ./ext-cpp/Inference.py ./ext-cpp/yolox_s.onnx Signs/"<<i<<".png > system.txt";
	tmp<<"python ./ext-cpp/Inference.py ./ext-cpp/yolox_s.onnx > system.txt";
	string execCmd = tmp.str();
	int a = system(execCmd.c_str());
	FILE *fp;
	int buff;

	fp = fopen("system.txt", "r");
	int result = fscanf(fp, "%d", &buff);
	fclose(fp);

	if(result == EOF)
		return -1;

	return buff;

}





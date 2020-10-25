#include "Types.h"
#include "Process.h"
#include "ParsingUtils.h"

int main(int argc, char* argv[]) {
    Arguments args = {(char*[3]){"-l", "-u", "-w"}, 3};
    char** usage_format = "./myprime -l lb -u ub -w NumofChildren";
    char **arg_values=NULL;
    int arg_values_size = -1;
    args_parser(argc, argv, &arg_values, &arg_values_size, args, args, args, "");


    // Now that we have the input.
    

}
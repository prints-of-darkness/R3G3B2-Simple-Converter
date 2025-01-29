/*

    RGB888 to RGB332 converter - public domain
    MJM + AI 2025
    no warranty implied; use at your own risk

*/

#include "options.h"
#include "fileio.h"
#include "image_process.h"

int main(int argc, char* argv[]) {
    ProgramOptions opts;
    init_program_options(&opts);
    if (parse_command_line_args(argc, argv, &opts)) {
        return 1;
    }
    return process_image(&opts);
}
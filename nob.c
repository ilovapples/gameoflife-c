#define NOB_IMPLEMENTATION
#include "nob.h"

#define BUILD_FOLDER "build/"
#define SRC_FOLDER   "src/"

int main(int argc, char* argv[]) {
	NOB_GO_REBUILD_URSELF(argc, argv);

	if (!nob_mkdir_if_not_exists(BUILD_FOLDER)) return 1;

	Nob_Cmd cmd = {0};

	nob_cmd_append(&cmd, "gcc", 
			"-Wall", "-Wextra", "-Werror", 
			"-o", BUILD_FOLDER"life", 
			SRC_FOLDER"life.c");

	if (!nob_cmd_run_sync_and_reset(&cmd)) return 1;
	cmd.count = 0;

	return 0;
}

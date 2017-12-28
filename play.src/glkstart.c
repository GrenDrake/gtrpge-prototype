/* glkstart.c: Unix-specific startup code -- sample file.
    Designed by Andrew Plotkin <erkyrath@eblong.com>
    http://www.eblong.com/zarf/glk/index.html

    This is Unix startup code for the simplest possible kind of Glk
    program -- no command-line arguments; no startup files; no nothing.

    Remember, this is a sample file. You should copy it into the Glk
    program you are compiling, and modify it to your needs. This should
    *not* be compiled into the Glk library itself.
*/

#include <string.h>

#include "glk.h"
#include "glkstart.h"

glkunix_argumentlist_t glkunix_arguments[] = {
    { "", glkunix_arg_ValueFollows, "filename: The game file to load." },
    { NULL, glkunix_arg_End, NULL }
};

extern char gamefile[64];

int glkunix_startup_code(glkunix_startup_t *data) {
    if (data->argc > 1) {
        strncpy(gamefile, data->argv[1], 63);
        gamefile[63] = 0;
    }
    glkunix_set_base_file(gamefile);
    return TRUE;
}


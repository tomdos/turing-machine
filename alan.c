#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Representation of turing machine with all inputs from machine's definition file */
typedef struct machine
{
    char *tape;
    size_t tape_size;
    size_t tape_idx;
    uint32_t current_state;
    uint32_t stop_state;
    char **action_table;

} machine_t;


static void
print_tape(machine_t *machine)
{
    int i;

    for (i = 0; i < machine->tape_size; i++)
        printf("%c", machine->tape[i]);

    if (i > 0)
        printf("\n");
}


#define readinput_fail(msg)     \
        {                       \
            perror(msg);        \
            if (fd)             \
                fclose(fd);     \
            if (machine)        \
                free(machine);  \
            return NULL;        \
        }

/*
 * Read an input file and initialize machine_t structure.
 */
static machine_t *
readinput(const char *filename)
{
    FILE *fd = NULL;
    machine_t *machine = NULL;
    ssize_t len;

    fd = fopen(filename, "r");
    if (fd == NULL)
        readinput_fail("Can not open input file");

    machine = (machine_t *) calloc(1, sizeof(machine_t));
    if (machine == NULL)
        readinput_fail("Malloc failed");


    /*
     * Parse initial states of tape
     *  - we have a content and we are happy
     *  - we don't have a content therefore initialize tape with '-' and
     *    check offset.
     */
    //machine->tape=malloc(10);machine->tape_size=10;
    len = getline(&(machine->tape), &(machine->tape_size), fd);
    if (len == -1)
        readinput_fail("Unable to read input");


    //printf("%zd %zd\n", machine->tape_size, len);
    //FIXME - no content!
    while (len <= machine->tape_size) {
        machine->tape[len-1] = '-';
        len++;
    }

    // Offset - compare to machine->tape_size
    // resize tape if offset is higher
    


    fclose(fd);
    return machine;
}



int
main(int argc, char *argv[])
{
    machine_t *machine;

    if (argc != 2) {
        fprintf(stderr, "Input file with machine's definition is required.\n");
        return 1;
    }

    if ((machine = readinput(argv[1])) == NULL) {
        fprintf(stderr, "Unable to parse machine's definition.\n");
        return 1;
    }



    return 0;
}

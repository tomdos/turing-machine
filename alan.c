/* 
 * Author: Tomas Dosoudil 
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <string.h>

/* Default value for tape resizing */
#define     MACHINE_TAPE_RESIZE             1

/* Default value for action table realloc */
#define     MACHINE_AT_REALLOC              20



/* Representation of state in action table */
typedef struct machine_state
{
    uint32_t state;
    uint32_t new_state;
    int32_t direction;
    char read;
    char write;
} machine_state_t;


/* Representation of turing machine with all inputs from machine's definition file */
typedef struct machine
{
    char *tape;
    size_t tape_size;
    uint32_t tape_idx;
    uint32_t current_state;
    uint32_t stop_state;
    machine_state_t **action_table;
    size_t action_table_size;
} machine_t;



/*
 * Print whole tape.
 */
static void
machine_tape_print(machine_t *machine)
{
    int i;

    for (i = 0; i < machine->tape_size; i++)
        printf("%c", machine->tape[i]);

    if (i > 0)
        printf("\n");
}


/*
 * Print content of action table.
 */
static void
machine_action_table_print(machine_t *machine)
{
    int i;
    machine_state_t *state;

    for (i = 0; i < machine->action_table_size; i++) {
        state = machine->action_table[i];

        printf("%u %c %c %d %u\n",
            state->state,
            state->read,
            state->write,
            state->direction,
            state->new_state);
    }
}


/*
 * Machine dump - verbose version for debugging purpose.
 */
static void
machine_print_dbg(machine_t *machine)
{
    machine_tape_print(machine);
    printf("size: %d\n", machine->tape_size);
    printf("idx: %d\n", machine->tape_idx);
    printf("state: %d\n", machine->current_state);
    printf("stop: %d\n", machine->stop_state);
    machine_action_table_print(machine);
}


/*
 * Print tape, head position and state.
 */
static void
machine_print(machine_t *machine)
{
    int i;

    printf("%2d|", machine->current_state);
    machine_tape_print(machine);
    printf("  |");
    for (i = 0; i < machine->tape_idx; i++)
        printf(" ");
    printf("^\n");
}


/*
 * Print content of the tape as required in assignment.
 */
static void
machine_last_line_print(machine_t *machine)
{
    int i;

    printf("+-------------------------------------------------------------+\n");
    for (i = 0; i < machine->tape_size; i++)
        printf("%c", machine->tape[i]);
    printf("\n");
}


/*
 * Final clean up of main structure.
 */
static void
machine_fini(machine_t *machine)
{
    int i;

    if (machine != NULL) {
        if (machine->tape)
            free(machine->tape);

        if (machine->action_table) {
            for (i = 0; i < machine->action_table_size; i++)
                free(machine->action_table[i]);

            free(machine->action_table);
        }

        free(machine);
    }
}


/*
 * Resize the tape and copy a content of old tape at offset of new tape.
 */
static int
machine_tape_resize(machine_t *machine, uint32_t offset, size_t size)
{
    char *new_tape;
    uint32_t i;

    assert(machine->tape_size + offset <= size);

    new_tape = (char *) malloc(sizeof(char) * size);
    assert(new_tape);

    /* Initialize new tape with a blank symbol. */
    for (i = 0; i < size; i++)
        new_tape[i] = '-';

    memcpy(new_tape + offset, machine->tape, machine->tape_size);

    free(machine->tape);

    machine->tape_size = size;
    machine->tape = new_tape;

    return 0;
}


/*
 * Move head to a new position - resize tape buffer if neccessary.
 * Direction can be: -1, 0, 1 for left, stay, right
 */
static int
machine_head_move(machine_t *machine, int direction)
{
    int offset;

    /* don't move or stay in range */
    if (direction == 0 ||
        ((machine->tape_idx + direction) >= 0 &&
        (machine->tape_idx + direction) < machine->tape_size)) {
        machine->tape_idx += direction;
        return 0;
    }

    /* resize tape buffer */
    if (direction < 0) {
        offset = MACHINE_TAPE_RESIZE;
        machine->tape_idx = offset - 1;
    }
    else {
        offset = 0;
        machine->tape_idx++;
    }

    return machine_tape_resize(machine, offset, machine->tape_size + MACHINE_TAPE_RESIZE);
}


/*
 * Find new state base on current state and current symbol on the tape.
 *
 * There might be several way how to optimize following code. However I assume
 * "reasonable" size of action table and therefor I iterate through all states.
 */
static int
machine_get_next_state(machine_t *machine)
{
    int state_idx = -1;
    machine_state_t *state;
    int i;

    for (i = 0; i < machine->action_table_size; i++) {
        /* search only among curent states */
        if (machine->action_table[i]->state != machine->current_state)
            continue;

        /* symbol on the tape must match symbol in state or state contains '*' */
        if (machine->action_table[i]->read == machine->tape[machine->tape_idx]
            || machine->action_table[i]->read == '*') {
            state_idx = i;
            break;
        }
    }

    return state_idx;
}


/*
 * Main loop processing the tape.
 */
static int
machine_run(machine_t *machine)
{
    int idx;
    machine_state_t *cs;

    while (1) {
        machine_print(machine);
        
        if (machine->current_state == machine->stop_state)
            break;        

        /* Find new states */
        idx = machine_get_next_state(machine);
        if (idx == -1)
            return 1;
        cs = machine->action_table[idx];

        /* Write new symbol */
        if (cs->write != '*')
            machine->tape[machine->tape_idx] = cs->write;

        /* Move head */
        if (machine_head_move(machine, cs->direction)) {
            fprintf(stderr, "Can not change possition of head.");
            return 1;
        }

        /* Set new state */
        machine->current_state = cs->new_state;
    }

    return 0;
}


/*
 * Read symbols and initialize the tape. Set size of the tape.
 */
static int
readinput_tape(FILE *fd, machine_t *machine)
{
    int len;

    machine->tape_size = MACHINE_TAPE_RESIZE;
    machine->tape = (char *) malloc(sizeof(char) * MACHINE_TAPE_RESIZE);
    assert(machine->tape);

    len = getline(&(machine->tape), &(machine->tape_size), fd);
    if (len == -1)
        return 1;

    /*
     * Fill rest of the tape with a blank symbol - last two characters
     * '\n' and '\0' will be also replaced.
     */
    while (len <= machine->tape_size) {
        machine->tape[len-1] = '-';
        len++;
    }

    return 0;
}


/*
 * Read and set offset (tape_idx). It may happend that the tape will be resized.
 */
static int
readinput_offset(FILE *fd, machine_t *machine)
{
    int32_t offset;
    uint32_t start;
    size_t size;
    int ret;

    ret = fscanf(fd, "%d\n", &offset);
    if (ret != 1 || errno != 0)
        return 1;

    if (offset < 0 || offset > machine->tape_size) {
        if (offset < 0) {
            start = abs(offset);
            size = start + machine->tape_size;
            machine->tape_idx = 0;
        }
        else {
            start = 0;
            size = offset;
            machine->tape_idx = offset;
        }

        machine_tape_resize(machine, start, size);
    }
    else {
        machine->tape_idx = offset;
    }

    return 0;
}

/*
 * Read a state (start and stop) from file.
 */
static int
readinput_state(FILE *fd, uint32_t *state)
{
    int ret;
    uint32_t v;

    ret = fscanf(fd, "%u\n", &v);
    if (ret != 1 || errno != 0)
        return 1;

    *state = v;

    return 0;
}


/*
 * Qsort comparator - sort states in action table (by state number and '*').
 */
static int
readinput_action_table_sortcmp(const void *param1, const void *param2)
{
    const machine_state_t *a = *(const machine_state_t **) param1;
    const machine_state_t *b = *(const machine_state_t **) param2;

    if (a->state < b->state)
        return -1;

    if (a->state > b->state)
        return 1;

    if (a->read == '*')
        return 1;

    if (b->read == '*')
        return -1;

    return 0;
}


/*
 * Build action table based on values in file.
 */
static int
readinput_action_table(FILE *fd, machine_t *machine)
{
    machine_state_t *state;
    uint32_t line;
    int n;

    line = 0;
    machine->action_table_size = 0;

    while (!feof(fd)) {
        state = (machine_state_t *) malloc(sizeof(machine_state_t));
        assert(state);

        n = fscanf(fd, "%u %c %c %d %u\n", &state->state, &state->read,
            &state->write, &state->direction, &state->new_state);

        if (n != 5 || errno != 0) {
            free(state);
            /* It may happend on some systems that EOF is reached after last line */
            if (feof(fd))
                break;

            return 1;
        }
        
        /* resize action table if there is no enough space for new state */
        if (line == machine->action_table_size) {
            machine->action_table_size += MACHINE_AT_REALLOC;
            machine->action_table = (machine_state_t **) realloc(machine->action_table,
                sizeof(machine_state_t *) * machine->action_table_size);
            assert(machine->action_table);
            memset(&(machine->action_table[line]), 0, MACHINE_AT_REALLOC);
        }        

        machine->action_table[line] = state;
        line++;
    }

    /* If file doesn't contain any state then faile. */
    if (machine->action_table_size == 0)
        return 1;

    /* Keep table size and not size of alloc'd memory */
    machine->action_table_size = line;

    /*
     * Action table has to be sorted.
     * Sort it by state number (wild-card must be the last rule within
     * the same state number).
     */
    qsort(machine->action_table, machine->action_table_size,
        sizeof(machine_state_t *), readinput_action_table_sortcmp);

    return 0;
}


/*
 * Clean up if readinput() fail.
 */
#define readinput_fail(msg)                         \
        {                                           \
            if (errno)                              \
                perror(msg);                        \
            else                                    \
                fprintf(stderr, msg".\n");          \
            if (fd)                                 \
                fclose(fd);                         \
            machine_fini(machine);                  \
            return NULL;                            \
        }


/*
 * Read an input file and initialize machine_t structure.
 * Consists of:
 *  - tape initialization
 *  - head offset
 *  - start and stop state
 *  - action table
 */
static machine_t *
readinput(const char *filename)
{
    FILE *fd = NULL;
    machine_t *machine = NULL;

    fd = fopen(filename, "r");
    if (fd == NULL)
        readinput_fail("Can not open input file");

    machine = (machine_t *) calloc(1, sizeof(machine_t));
    if (machine == NULL)
        readinput_fail("Malloc failed");

    /*
     * Read initial
     *  - symbols on the tape
     *  - offset
     *  - states (start, stop)
     *  - action table
     */
    if (readinput_tape(fd, machine))
        readinput_fail("Unable to initialize the")

    /* Read offset only if the tape contains something */
    if (machine->tape_size > 1) {
        if (readinput_offset(fd, machine))
            readinput_fail("Unable to set offset");
    }

    if (readinput_state(fd, &machine->current_state))
        readinput_fail("Unable to set start state");

    if (readinput_state(fd, &machine->stop_state))
        readinput_fail("Unable to set halting state");

    if (readinput_action_table(fd, machine))
        readinput_fail("Unable to set action table")

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

    machine_print_dbg(machine);

    if (machine_run(machine)) {
        fprintf(stderr, "Unable to process this definition of Turing machine.\n");
        machine_fini(machine);
        return 1;
    }

    machine_last_line_print(machine);
    machine_fini(machine);

    return 0;
}

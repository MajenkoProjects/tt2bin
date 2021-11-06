#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>

struct rom {
    char *name;
    int inputs;
    int outputs;
};

struct rom roms[] = {
    { "39FS010A", 17, 8 },
    { "39FS020A", 18, 8 },
    { "39FS040", 19, 8 },
    { 0, 0, 0 },
};


// tt2bin fromfile.txt tofile.bin --a0=blah --a1=foo --d0=um --d1=whatever {etc}
int main(int argc, char **argv) {

    char *apins[20] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    char *dpins[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    char *cols[28] = { 0 };

    static struct option long_options[] = {
        {"a0",      required_argument,      0,      0 },
        {"a1",      required_argument,      0,      0 },
        {"a2",      required_argument,      0,      0 },
        {"a3",      required_argument,      0,      0 },
        {"a4",      required_argument,      0,      0 },
        {"a5",      required_argument,      0,      0 },
        {"a6",      required_argument,      0,      0 },
        {"a7",      required_argument,      0,      0 },
        {"a8",      required_argument,      0,      0 },
        {"a9",      required_argument,      0,      0 },
        {"a10",     required_argument,      0,      0 },
        {"a11",     required_argument,      0,      0 },
        {"a12",     required_argument,      0,      0 },
        {"a13",     required_argument,      0,      0 },
        {"a14",     required_argument,      0,      0 },
        {"a15",     required_argument,      0,      0 },
        {"a16",     required_argument,      0,      0 },
        {"a17",     required_argument,      0,      0 },
        {"a18",     required_argument,      0,      0 },
        {"a19",     required_argument,      0,      0 },
        {"d0",      required_argument,      0,      0 },
        {"d1",      required_argument,      0,      0 },
        {"d2",      required_argument,      0,      0 },
        {"d3",      required_argument,      0,      0 },
        {"d4",      required_argument,      0,      0 },
        {"d5",      required_argument,      0,      0 },
        {"d6",      required_argument,      0,      0 },
        {"d7",      required_argument,      0,      0 },
        {"rom",     required_argument,      0,      0 },
        {0,         0,                      0,      0 },
    };

    int assigned = 0;

    int i, j;
    int c;
    int option_index = 0;

    struct rom *selectedRom = NULL;

    while ((c = getopt_long_only(argc, argv, "", long_options, &option_index)) != -1) {

        if (c == 0) {
            if (option_index < 20) {
                apins[option_index] = optarg;
            } else if (option_index < 28) {
                dpins[option_index - 20] = optarg;
            } else if (option_index == 28) {
                for (struct rom *r = roms; r->name != NULL; r++) {
                    if (strcasecmp(r->name, optarg) == 0) {
                        selectedRom = r;
                    }
                }
                if (selectedRom == NULL) {
                    printf("Error: unknown ROM code %s. Select from:\n", optarg);
                    for (struct rom *r = roms; r->name != NULL; r++) {
                        printf("  %s\n", r->name);
                    }
                }
            }
        }
    }


    if (argc - optind != 2) {
        printf("Usage: tt2bin [options] input.txt output.bin\n");
        return -1;
    }

    char *infile = argv[optind];
    char *outfile = argv[optind + 1];


    if (selectedRom == NULL) {
        printf("No ROM selected.\n");
        return -1;
    }

    int capacity = 1 << selectedRom->inputs;

    printf("ROM capacity: %d bytes\n", capacity);

    printf("Inputs: \n");
    for (i = 0; i < selectedRom->inputs; i++) {
        printf("  %2d: %s\n", i, apins[i] != 0 ? apins[i] : "GND");
    }
    printf("Outputs: \n");
    for (i = 0; i < selectedRom->outputs; i++) {
        printf("  %2d: %s\n", i, dpins[i] != 0 ? dpins[i] : "0");
    }

    uint8_t romData[capacity];
    memset(romData, 0, capacity);

    FILE *in = fopen(infile, "r");
    if (!in) {
        printf("Error opening %s: %s\n", infile, strerror(errno));
        return -1;
    }

    char temp[4096];

    if (!fgets(temp, 4096, in)) {
        printf("Error reading from input file!\n");
        fclose(in);
        return -1;
    }

    i = 0;
    char *col = strtok(temp, "\t \r\n");
    while ((col) && (i < 28)) {
        cols[i++] = strdup(col);
        col = strtok(NULL, "\t \r\n");
    }

    #define INPUT_PIN 0x1000
    #define OUTPUT_PIN 0x2000

    int colmap[28] = { 0 };

    for (i = 0; i < selectedRom->inputs; i++) {
        if (apins[i] != 0) {
            int found = 0;
            for (j = 0; (j < 28) && (cols[j] != 0) && (found == 0); j++) {
                if (strcasecmp(apins[i], cols[j]) == 0) {
                    found = 1;
                    colmap[j] = INPUT_PIN | i;
                }
            }
            if (found == 0) {
                printf("Error: unknown signal: %s\n", apins[i]);
                fclose(in);
                return -1;
            }
        }
    }

    for (i = 0; i < selectedRom->outputs; i++) {
        if (dpins[i] != 0) {
            int found = 0;
            for (j = 0; (j < 28) && (cols[j] != 0) && (found == 0); j++) {
                if (strcasecmp(dpins[i], cols[j]) == 0) {
                    found = 1;
                    colmap[j] = OUTPUT_PIN | i;
                }
            }
            if (found == 0) {
                printf("Error: unknown signal: %s\n", dpins[i]);
                fclose(in);
                return -1;
            }
        }
    }


    while (fgets(temp, 4096, in)) {
        int colnum = 0;
        char *val = strtok(temp, "\t \r\n");

        uint32_t address = 0;
        uint8_t data = 0;

        while (val) {
            int map = colmap[colnum];
            if (map == 0) {
                printf("Oops, something wrong with the file format.\n");
                fclose(in);
                return -1;
            }

            if (map & INPUT_PIN) {
                map &= 0xFFFF;
                if (val[0] == '1') {
                    address |= 1 << map;
                }
            }

            if (map & OUTPUT_PIN) {
                map &= 0xFFFF;
                if (val[0] == '1') {
                    data |= 1 << map;
                }
            }
            
            colnum++;
            val = strtok(NULL, "\t \r\n");
        }

        if (address >= capacity) {
            printf("Error: Rom too small?!\n");
            fclose(in);
        }

        romData[address] = data;
    }

    fclose(in);

    int out = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (!out) {
        printf("Unable to open %s for writing: %s\n", outfile, strerror(errno));
        return -1;
    }

    write(out, romData, capacity);
    close(out);

    return 0;
}

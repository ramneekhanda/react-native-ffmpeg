//
// Created by ramneek on 13/6/17.
//

#include "../make_gif.h"

int main(int argc, char **argv) {
    if (argc != 3) {
        return 1;
    }
    return make_gif(argv[1], argv[2]);
}

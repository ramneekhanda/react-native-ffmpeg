//
// Created by ramneek on 13/6/17.
//

#include "../encodeVideoOnly.h"
#include <libavcodec/avcodec.h>

#define NULL ((void*)0)

void progressImpl(void *vp, int val) {
}

void doneImpl(void *vp) {
}

void errorImpl(void *vp, char *err) {
}

int main(int argc, char **argv) {
    if (argc != 3) {
        return 1;
    }
    encodeVideoOnly(argv[1], argv[2], NULL, "gif", "null", progressImpl, doneImpl, errorImpl);
}

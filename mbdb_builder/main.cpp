//
//  main.cpp
//  mbdb_builder
//
//  Created by FengXing on 11/24/14.
//  Copyright (c) 2014 fengxing. All rights reserved.
//

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mbdbdump.h"
bool g_readonly = false;

int main(int argc, const char * argv[]) {
    // insert code here...
    if (argc > 1) {
        argv++;
        if (strcmp(*argv, "r") == 0) {
            g_readonly = true;
            argv++;
        }
        return rebuild(*argv);
    }
    printf("%s [r] dir", *argv);
    return 1;
}

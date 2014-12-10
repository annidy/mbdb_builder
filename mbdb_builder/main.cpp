//
//  main.cpp
//  mbdb_builder
//
//  Created by FengXing on 11/24/14.
//  Copyright (c) 2014 fengxing. All rights reserved.
//

#include <iostream>
#include "mbdbdump.h"

int main(int argc, const char * argv[]) {
    // insert code here...
    if (argc > 1)
        return rebuild(argv[1]);
    return 0;
}

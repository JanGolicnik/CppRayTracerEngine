#pragma once

#include "core.h"
#include "BaseTypes.h"

namespace MyPBRT {

    struct Object {
        int shape = 0;
        int material = 0;
        
        Object(int shape, int material);
        Object() {}

        //true if scene should update
        bool CreateIMGUI(const std::vector<std::shared_ptr<Material>>& materials);
    };

}


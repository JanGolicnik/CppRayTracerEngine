#pragma once

#include "core.h"
#include "BaseTypes.h"

namespace MyPBRT {

    struct Object {
        int shape;
        int material;
        
        Object(int shape, int material);

        //true if scene should update
        bool CreateIMGUI(const std::vector<std::shared_ptr<Material>>& materials);
    };

}


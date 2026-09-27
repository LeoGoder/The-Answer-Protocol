#include "engine.hpp"
#include "tap.hpp"
#include <unistd.h>

int main(int argc, char **argv) {
    Tyra::EngineOptions options;
    // guys set it to true before compiling for the real console find it in tap.hpp
    if (Tyra::IS_REAL_PS2_VIA_USB) {
        options.writeLogsToFile = true;
        options.loadUsbDriver = true;
    }
    Tyra::Engine engine(options);
    sleep(3);
    Tyra::Tap game(&engine);
    engine.run(&game);
    return 0;
}

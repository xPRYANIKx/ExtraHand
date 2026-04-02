#define NX_SERVICE_ASSUME_NON_DOMAIN
#define TESLA_INIT_IMPL

#include "app_overlay.hpp"
#include "app_utils.hpp"

int main(int argc, char** argv) {


    if (argc > 0 && argv != nullptr && argv[0] != nullptr) {

        g_selfOverlayFileName = getFileNameFromPath(argv[0]);
    }

    return tsl::loop<ExtraHandOverlay>(argc, argv);
}
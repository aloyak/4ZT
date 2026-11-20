#include <4ZTScout/scout.h>

int main() {

    Scout::App app;

    while (app.IsRunning()) {
        app.Run();
    }

}
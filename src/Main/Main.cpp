#include "Main.h"
#include <Core/File.h>
#include <Core/Print.h>
#include <Ty/System.h>

int main(int argc, c_string argv[])
{
    while (true) {
        auto result = Main::main(argc, argv);
        if (result.is_error()) {
            Core::File::stderr()
                .writeln("Error: "sv, result.error())
                .ignore();
            dbgln("Restarting in 10 seconds\n"sv);
            System::sleep(10);
            dbgln("Restarting"sv);
            continue;
        }
        return result.value();
    }
}

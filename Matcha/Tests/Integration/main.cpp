#define DOCTEST_CONFIG_IMPLEMENT

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include "WidgetTestFixture.h"

auto main(int argc, char* argv[]) -> int {
    matcha::test::WidgetTestFixture fixture;

    doctest::Context context;
    context.applyCommandLine(argc, argv);
    return context.run();
}

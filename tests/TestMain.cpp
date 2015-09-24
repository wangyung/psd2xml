#include <gtest/gtest.h>

int g_argc;
char **g_argv;

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    g_argc = argc;
    g_argv = argv;
    return RUN_ALL_TESTS();
}

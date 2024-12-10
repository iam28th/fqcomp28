int startProgram(int argc, char **argv);

/**
 * a wrapper around the actual entry point, so that unittests
 * could be linked against fqzcomp28 object library
 */
int main(int argc, char **argv) { startProgram(argc, argv); }

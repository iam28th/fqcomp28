namespace fqzcomp28 {
int startProgram(int argc, char **argv);
}

/**
 * a wrapper around the actual entry point, so that unittests
 * could be linked against fqzcomp28 as an object library
 */
int main(int argc, char **argv) { fqzcomp28::startProgram(argc, argv); }

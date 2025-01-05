#include "app.h"
#include "process.h"
#include "settings.h"

namespace {
void addCommonOptions(CLI::App *subcommand);
CLI::App *createCompressSubcommand(CLI::App *app);
CLI::App *createDecompressSubcommand(CLI::App *app);
} // namespace

namespace fqzcomp28 {

void addOptions(CLI::App *app) {
  app->require_subcommand(1, 1);
  app->option_defaults()->always_capture_default();
  app->set_help_all_flag("--help-all", "Expand all help");

  auto *c = createCompressSubcommand(app);
  auto *d = createDecompressSubcommand(app);

  c->parse_complete_callback(processReads);
  d->parse_complete_callback(fqzcomp28::processArchiveParts);
}
} // namespace fqzcomp28

namespace {
void addCommonOptions(CLI::App *subcommand) {
  auto *set = fqzcomp28::Settings::getInstance();
  subcommand->add_option("-t,--threads", set->non_storable.n_threads,
                         "number of processing threads");
  subcommand->add_flag("--verbose", set->non_storable.verbose,
                       "produce verbose output");
}

CLI::App *createCompressSubcommand(CLI::App *app) {
  auto *set = fqzcomp28::Settings::getInstance();
  auto *cmd = app->add_subcommand("c", "run compression");

  cmd->add_option("--i1,--input1", set->non_storable.mates1,
                  "path to a fastq file with first mates")
      ->check(CLI::ExistingFile)
      ->required();
  cmd->add_option("--i2,--input2", set->non_storable.mates2,
                  "path to a fastq file with second mates")
      ->check(CLI::ExistingFile);

  cmd->add_option("-o,--output", set->non_storable.archive)->required();

  addCommonOptions(cmd);
  return cmd;
}

CLI::App *createDecompressSubcommand(CLI::App *app) {
  auto *set = fqzcomp28::Settings::getInstance();
  auto *cmd = app->add_subcommand("d", "run decompression");

  cmd->add_option("-i,--input", set->non_storable.archive,
                  "path to an archive to decompress")
      ->check(CLI::ExistingFile)
      ->required();

  auto *o1 = cmd->add_option(
      "--o1,--output1", set->non_storable.mates1,
      "path to output mates1 (if both --o1 and --o2 are ommitted, "
      "prints to stdout)");

  cmd->add_option("--o2,--output2", set->non_storable.mates2,
                  "path to output mates2")
      ->needs(o1);

  addCommonOptions(cmd);
  return cmd;
}
} // namespace

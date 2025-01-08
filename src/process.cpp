#include "app.h"
#include "archive.h"
#include "compressed_buffers.h"
#include "defs.h"
#include "fastq_io.h"
#include "settings.h"
#include "workspace.h"
#include <future>
#include <thread>
#include <vector>

namespace fqcomp28 {

/** fqcomp28 entry point */
int startProgram(int argc, char **argv) {
  CLI::App app;
  fqcomp28::addOptions(&app);

  int ret = 0;

  try {
    app.parse(argc, argv);
  } catch (const CLI::CallForHelp &e) {
    ret = app.exit(e);
  } catch (const CLI::ParseError &e) {
    ret = app.exit(e);
  }

  return ret;
}

void processReads() {
  const auto *set = Settings::getInstance();
  const auto mates1 = set->non_storable.mates1;

  Archive archive(set->non_storable.archive, mates1, set->sample_chunk_size());
  FastqReader reader(mates1, set->reading_chunk_size());

  const unsigned n_threads = set->non_storable.n_threads;
  std::vector<std::thread> threads;
  threads.reserve(n_threads);

  std::vector<std::promise<InputStats>> thread_input_stats(n_threads);
  std::vector<std::promise<CompressedStats>> thread_compressed_stats(n_threads);

  for (unsigned i = 0; i < n_threads; ++i) {
    auto &istat_promise = thread_input_stats[i];
    auto &cstat_promise = thread_compressed_stats[i];
    threads.emplace_back([&reader, &archive, &istat_promise, &cstat_promise]() {
      FastqChunk chunk;
      InputStats istats;

      CompressedBuffersDst cbs;
      CompressionWorkspace wksp(&archive.meta());

      while (reader.readNextChunk(chunk)) {
        istats.seq += chunk.tot_reads_length;
        istats.header += chunk.headers_length;
        istats.n_records += chunk.records.size();

        wksp.encodeChunk(chunk, cbs);
        archive.writeBlock(cbs);
      }

      istat_promise.set_value(istats);
      cstat_promise.set_value(wksp.stats());
    });
  }
  for (auto &t : threads)
    t.join();

  InputStats istats = thread_input_stats[0].get_future().get();
  CompressedStats cstats = thread_compressed_stats[0].get_future().get();
  for (unsigned i = 1; i < n_threads; ++i) {
    istats += thread_input_stats[i].get_future().get();
    cstats += thread_compressed_stats[i].get_future().get();
  }

  archive.writeIndex();
  archive.flush(); /* needed to compare reported CR against actual filesize */
  printReport(istats, cstats, archive, std::cerr);
}

void processArchiveParts() {
  auto *const set = Settings::getInstance();
  Archive archive(set->non_storable.archive);
  FastqWriter writer(set->non_storable.mates1);

  const unsigned n_threads = set->non_storable.n_threads;
  std::vector<std::jthread> threads;
  threads.reserve(n_threads);

  for (unsigned i = 0; i < n_threads; ++i) {
    threads.emplace_back([&archive, &writer]() {
      CompressedBuffersSrc cbs;
      FastqChunk chunk;
      DecompressionWorkspace wksp(&archive.meta());

      while (archive.readBlock(cbs)) {
        wksp.decodeChunk(chunk, cbs);
        writer.writeChunk(chunk);
      }
    });
  }
}
} // namespace fqcomp28

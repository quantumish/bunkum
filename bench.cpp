#include <benchmark/benchmark.h>

extern "C" {
	#include "http/request.h"
	#include <string.h>
	
	const char* method_names[] = {"GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE"};

	enum http_method method_naive_enum(char* p) {
		for (int i = 0; i < 8; i++) {
			if (strcmp(method_names[i], p) == 0) {
				return (enum http_method) i;
			}
		}
	}
}

static void BM_PHF(benchmark::State& state) {
	// Perform setup here
	for (auto _ : state) {
		// This code gets timed
		method_enum((char*)"OPTIONS");
	}
}

static void BM_Naive(benchmark::State& state) {
	// Perform setup here
	for (auto _ : state) {
		// This code gets timed
		method_naive_enum((char*)"OPTIONS");
	}
}

// Register the function as a benchmark
BENCHMARK(BM_PHF);
BENCHMARK(BM_Naive);
// Run the benchmark
BENCHMARK_MAIN();

#pragma once
#include <cstdint>

#define MAX_FILE_NAME_SIZE 255
#define ENCODER_FILE_EXTENSION ".huff"
#define MAGIC_CODE "HUFF"

#define BYTE_SIZE 256
#define ONE_KB 1024
#define ONE_MB ONE_KB *ONE_KB
#define BUFFER_SIZE 1024 * ONE_KB

typedef std::uint8_t TYPE_CHAR;
typedef std::uint64_t TYPE_FREQUENCY;
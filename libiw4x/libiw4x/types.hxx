#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <ios>
#include <iostream>
#include <memory>
#include <mutex>
#include <span>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace iw4x
{
  using std::byte;
  using std::size_t;

  using std::int8_t;
  using std::int16_t;
  using std::int32_t;
  using std::int64_t;

  using std::intmax_t;
  using std::intptr_t;

  using std::uint8_t;
  using std::uint16_t;
  using std::uint32_t;
  using std::uint64_t;

  using std::uintmax_t;
  using std::uintptr_t;

  using std::ios;
  using std::cerr;
  using std::cout;
  using std::endl;

  using std::string;

  using std::array;
  using std::span;
  using std::unordered_map;
  using std::vector;

  using std::function;
  using std::forward;

  using std::unique_ptr;
  using std::make_unique;

  using std::mutex;
  using std::scoped_lock;
  using std::once_flag;
  using std::call_once;

  using std::runtime_error;
  using std::invalid_argument;

  using std::is_function_v;
  using std::remove_pointer_t;
}

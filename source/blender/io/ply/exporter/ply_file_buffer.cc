/* SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup ply
 */

#include "ply_file_buffer.hh"

namespace blender::io::ply {

FileBuffer::FileBuffer(const char *filepath, size_t buffer_chunk_size)
    : buffer_chunk_size_(buffer_chunk_size), filepath_(filepath)
{
  outfile_ = BLI_fopen(filepath, "wb");
  if (!outfile_) {
    throw std::system_error(
        errno, std::system_category(), "Cannot open file " + std::string(filepath) + ".");
  }
}

void FileBuffer::write_to_file()
{
  for (const VectorChar &b : blocks_) {
    fwrite(b.data(), 1, b.size(), this->outfile_);
  }
  blocks_.clear();
}

void FileBuffer::close_file()
{
  int close_status = std::fclose(outfile_);
  if (close_status == EOF) {
    return;
  }
  if (outfile_ && close_status) {
    std::cerr << "Error: could not close the file '" << this->filepath_
              << "' properly, it may be corrupted." << std::endl;
  }
}

void FileBuffer::write_header_element(StringRef name, int count)
{
  write_fstring("element {} {}\n", name, count);
}
void FileBuffer::write_header_scalar_property(StringRef dataType, StringRef name)
{
  write_fstring("property {} {}\n", dataType, name);
}

void FileBuffer::write_header_list_property(StringRef countType,
                                            StringRef dataType,
                                            StringRef name)
{
  write_fstring("property list {} {} {}\n", countType, dataType, name);
}

void FileBuffer::write_string(StringRef s)
{
  write_fstring("{}\n", s);
}

void FileBuffer::write_newline()
{
  write_fstring("\n");
}

void FileBuffer::ensure_space(size_t at_least)
{
  if (blocks_.empty() || (blocks_.back().capacity() - blocks_.back().size() < at_least)) {
    VectorChar &b = blocks_.emplace_back(VectorChar());
    b.reserve(std::max(at_least, buffer_chunk_size_));
  }
}

void FileBuffer::write_bytes(Span<char> bytes)
{
  ensure_space(bytes.size());
  VectorChar &bb = blocks_.back();
  bb.insert(bb.end(), bytes.begin(), bytes.end());
}

}  // namespace blender::io::ply

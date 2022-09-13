#include <cstdint>
#include <cstdio>
#include <cstring>
#include <elf.h>
#include <memory>
#include <utility>

int main(int argc, char **argv) {
  if (argc != 3) {
    fputs("Usage: bfc <input-file> <output-file>", stderr);
    return 1;
  }
  FILE *input_file = std::fopen(argv[1], "r");
  if (!input_file) {
    fputs("Could not open input file", stderr);
    return 1;
  }
  FILE *output_file = std::fopen(argv[2], "wb");
  if (!output_file) {
    fputs("Could not open output file", stderr);
    return 1;
  }

  const auto put_value = [&](auto value) {
    fwrite(&value, sizeof(value), 1u, output_file);
  };
  const auto put8 = [&](std::uint8_t value) { put_value(value); };
  const auto put16 = [&](std::uint16_t value) { put_value(value); };
  const auto put32 = [&](std::uint32_t value) { put_value(value); };

  // write the elf header

  static const char elf_magic[] = {0x7F, 'E', 'L', 'F'};
  std::fwrite(elf_magic, sizeof(char), 4, output_file);
  put8(1);                              // 32 bit target
  put8(1);                              // little endian
  put8(1);                              // current ELF version
  put8(3);                              // Linux OS ABI
  put8(0);                              // ABI version
  std::fseek(output_file, 7, SEEK_CUR); // skip padding bytes
  put16(1);                             // Relocatable file
  put16(0x28);                          // Target instruction set is ARM
  put32(1);                             // ELF version. Always 1
  put32(0);                             // Entry point
  put32(0);                             // Program header table offset
  const auto section_header_table_offset = std::ftell(output_file);
  put32(0);          // offset of first section header
  put32(0x05000000); // flags for the architecture: arm eabi ver 5
  put16(0x34);       // size of this header
  put16(0);          // program header entry size
  put16(0);          // program header count
  put16(0x28);       // section header size
  put16(5);          // section count
  put16(4);          // section name string table section

  // write the .text section
  const auto text_section_offset = std::ftell(output_file);
  for (char c = std::getc(input_file); c != EOF; c = std::getc(input_file)) {
    switch (c) {
    case '<':
      put16(0b0001111001000000); // sub r0, r0, #1
      break;
    case '>':
      put16(0b0001110001000000); // add r0, r0, #1
      break;
    case '+':
      put16(0b0111100000000001); // ldrb r1, [r0]
      put16(0b0001110001001001); // add  r0, r0, #1
      put16(0b0111000000000001); // strb r1, [r0]
      break;
    case '-':
      put16(0b0111100000000001); // ldrb r1, [r0]
      put16(0b0001111001001001); // add r0, r0, #1
      put16(0b0111000000000001); // strb r1, [r0]
      break;
    case ',':
      put16(0b0100011110011000); // blx r3
      break;
    case '.':
      put16(0b0100011110100000); // blx r4
      break;
    }
  }
  put16(0b0100011101110000); // bx lr
  const auto text_section_end_offset = std::ftell(output_file);

  // write .strtab
  const auto strtab_section_offset = std::ftell(output_file);
  put8(0);
  const auto file_name_offset = std::ftell(output_file);
  std::fwrite(argv[1], sizeof(char), std::strlen(argv[1]) + 1, output_file);

  const auto text_section_sym_name_offset = std::ftell(output_file);
  static const char text_section_name[] = ".text";
  std::fwrite(text_section_name, sizeof(char), sizeof(text_section_name),
              output_file);

  const auto main_function_name_offset = std::ftell(output_file);
  static const char main_function_name[] = "main";
  std::fwrite(main_function_name, sizeof(char), sizeof(main_function_name),
              output_file);

  const auto strtab_section_end_offset = std::ftell(output_file);

  // write .symtab
  const auto symtab_section_offset = std::ftell(output_file);

  // write zero symbol
  put32(0);
  put32(0);
  put32(0);
  put8(0);
  put8(0);
  put16(0);

  // write file symbol
  put32(file_name_offset - strtab_section_offset);
  put32(0);
  put32(0);
  put8(0b00000100); // STB_LOCAL, STT_FILE
  put8(0);
  put16(0);

  // write .text symbol
  put32(text_section_sym_name_offset - strtab_section_offset);
  put32(0);
  put32(text_section_end_offset - text_section_offset);
  put8(0b00000011); // STB_LOCAL, STT_SECTION
  put8(0);
  put16(0);

  // write main symbol
  put32(main_function_name_offset - strtab_section_offset);
  put32(0 + 1); // add 1 to indicate that this is thumb code
  put32(text_section_end_offset - text_section_offset);
  put8(0b00010010); // STB_GLOBAL, STT_FUNC
  put8(0);
  put16(1);

  const auto symtab_section_end_offset = std::ftell(output_file);

  // write .shstrtab
  const auto shstrtab_section_offset = std::ftell(output_file);
  put8(0);
  const auto text_section_name_offset = std::ftell(output_file);
  std::fwrite(text_section_name, sizeof(char), sizeof(text_section_name),
              output_file);
  static const char strtab_section_name[] = ".strtab";
  const auto strtab_section_name_offset = std::ftell(output_file);
  std::fwrite(strtab_section_name, sizeof(char), sizeof(strtab_section_name),
              output_file);
  static const char symtab_section_name[] = ".symtab";
  const auto symtab_section_name_offset = std::ftell(output_file);
  std::fwrite(symtab_section_name, sizeof(char), sizeof(symtab_section_name),
              output_file);
  static const char shstrtab_section_name[] = ".shstrtab";
  const auto shstrtab_section_name_offset = std::ftell(output_file);
  std::fwrite(shstrtab_section_name, sizeof(char),
              sizeof(shstrtab_section_name), output_file);

  const auto shstrtab_section_end_offset = std::ftell(output_file);

  // Writing section headers
  const auto section_header_offset = std::ftell(output_file);
  // Patch the section header table offset in the header
  std::fseek(output_file, section_header_table_offset, SEEK_SET);
  put32(section_header_offset);
  std::fseek(output_file, section_header_offset, SEEK_SET);

  // write null section header
  put32(0); // section name
  put32(0); // section type null
  put32(0); // no flags
  put32(0); // virtual address of the section
  put32(0); // offset of the section in the file image
  put32(0); // size of the section in the file image
  put32(0); // linked section
  put32(0); // extra info for the section
  put32(1); // section alignment
  put32(0); // section entry size

  // write .text section header
  put32(text_section_name_offset - shstrtab_section_offset); // section name
  put32(0x1);                 // section type progbits
  put32(0x2 | 0x4);           // alloc and exec
  put32(0);                   // Virtual address of the section
  put32(text_section_offset); // offset of the section in the file image
  put32(text_section_end_offset -
        text_section_offset); // size of the section in bytes in the file image
  put32(0);                   // section link
  put32(0);                   // section info
  put32(2);                   // section alignment
  put32(0);                   // section has no entrysize

  // write .strtab section header
  put32(strtab_section_name_offset - shstrtab_section_offset);
  put32(0x3);                   // section type strtab
  put32(0x20);                  // section contains strings
  put32(0);                     // virtual address of the section
  put32(strtab_section_offset); // offset of the section in the file image
  put32(strtab_section_end_offset -
        strtab_section_offset); // size of the section in the file image
  put32(0);                     // section link
  put32(0);                     // section info
  put32(1);                     // section align
  put32(0);                     // entrysize

  // write .symtab section header
  put32(symtab_section_name_offset - shstrtab_section_offset);
  put32(0x2);                   // section type symtab
  put32(0);                     // no flags
  put32(0);                     // no virtual address of the section
  put32(symtab_section_offset); // offset of the section in bytes in the file
                                // image
  put32(symtab_section_end_offset -
        symtab_section_offset); // size of the section in the file image
  put32(2);                     // section link - reference .strtab
  put32(3);                     // section info - reference first global symbol
  put32(1);                     // section alignment
  put32(0x10);                  // symtab entry size

  // write .shstrtab section header
  put32(shstrtab_section_name_offset - shstrtab_section_offset);
  put32(0x3);                     // section type strtab
  put32(0x20);                    // section contains strings
  put32(0);                       // virtual address of the section
  put32(shstrtab_section_offset); // offset of the section in the file image
  put32(shstrtab_section_end_offset -
        shstrtab_section_offset); // size of the section in the file image
  put32(0);                       // section link
  put32(0);                       // section info
  put32(1);                       // section alignment
  put32(0);                       // section has no entry size

  fclose(input_file);
  fclose(output_file);

  return 0;
}

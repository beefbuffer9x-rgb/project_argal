#pragma once
#include <cstdint>
#include <vector>
#include <random>
#include <algorithm>

namespace opcode {

    constexpr uint8_t SAFE_REGS[] = { 0, 1, 2, 3, 6, 7 }; // RAX, RCX, RDX, RBX, RSI, RDI
    constexpr int NUM_SAFE_REGS = 6;

    inline uint8_t rand_reg(std::mt19937& rng) {
        return SAFE_REGS[rng() % NUM_SAFE_REGS];
    }

    inline uint8_t rand_byte(std::mt19937& rng) {
        return static_cast<uint8_t>(rng() & 0xFF);
    }

    inline uint32_t rand_dword(std::mt19937& rng) {
        return static_cast<uint32_t>(rng());
    }

    // ModR/M for register-register (mod=11)
    inline uint8_t modrm_rr(uint8_t reg, uint8_t rm) {
        return 0xC0 | ((reg & 7) << 3) | (rm & 7);
    }

    // 1-byte
    inline void emit_nop(std::vector<uint8_t>& o) { o.push_back(0x90); }
    inline void emit_push(std::vector<uint8_t>& o, uint8_t r) { o.push_back(0x50 + (r & 7)); }
    inline void emit_pop(std::vector<uint8_t>& o, uint8_t r) { o.push_back(0x58 + (r & 7)); }

    // 2-byte
    inline void emit_test_r32(std::vector<uint8_t>& o, uint8_t r1, uint8_t r2) {
        o.push_back(0x85); o.push_back(modrm_rr(r2, r1));
    }
    inline void emit_xor_r32(std::vector<uint8_t>& o, uint8_t r1, uint8_t r2) {
        o.push_back(0x31); o.push_back(modrm_rr(r2, r1));
    }
    inline void emit_mov_r32(std::vector<uint8_t>& o, uint8_t dst, uint8_t src) {
        o.push_back(0x89); o.push_back(modrm_rr(src, dst));
    }
    inline void emit_cmp_r32(std::vector<uint8_t>& o, uint8_t r1, uint8_t r2) {
        o.push_back(0x39); o.push_back(modrm_rr(r2, r1));
    }
    inline void emit_and_r32(std::vector<uint8_t>& o, uint8_t r1, uint8_t r2) {
        o.push_back(0x21); o.push_back(modrm_rr(r2, r1));
    }
    inline void emit_or_r32(std::vector<uint8_t>& o, uint8_t r1, uint8_t r2) {
        o.push_back(0x09); o.push_back(modrm_rr(r2, r1));
    }
    inline void emit_sub_r32(std::vector<uint8_t>& o, uint8_t r1, uint8_t r2) {
        o.push_back(0x29); o.push_back(modrm_rr(r2, r1));
    }
    inline void emit_add_r32(std::vector<uint8_t>& o, uint8_t r1, uint8_t r2) {
        o.push_back(0x01); o.push_back(modrm_rr(r2, r1));
    }

    // 3-byte  REX.W + op r64, r64
    inline void emit_mov_r64(std::vector<uint8_t>& o, uint8_t dst, uint8_t src) {
        o.push_back(0x48); o.push_back(0x89); o.push_back(modrm_rr(src, dst));
    }
    inline void emit_xor_r64(std::vector<uint8_t>& o, uint8_t r1, uint8_t r2) {
        o.push_back(0x48); o.push_back(0x31); o.push_back(modrm_rr(r2, r1));
    }
    inline void emit_test_r64(std::vector<uint8_t>& o, uint8_t r1, uint8_t r2) {
        o.push_back(0x48); o.push_back(0x85); o.push_back(modrm_rr(r2, r1));
    }
    inline void emit_add_r64(std::vector<uint8_t>& o, uint8_t r1, uint8_t r2) {
        o.push_back(0x48); o.push_back(0x01); o.push_back(modrm_rr(r2, r1));
    }
    inline void emit_sub_r64(std::vector<uint8_t>& o, uint8_t r1, uint8_t r2) {
        o.push_back(0x48); o.push_back(0x29); o.push_back(modrm_rr(r2, r1));
    }
    inline void emit_cmp_r64(std::vector<uint8_t>& o, uint8_t r1, uint8_t r2) {
        o.push_back(0x48); o.push_back(0x39); o.push_back(modrm_rr(r2, r1));
    }
    inline void emit_lea_r64_r64(std::vector<uint8_t>& o, uint8_t dst, uint8_t src) {
        o.push_back(0x48); o.push_back(0x8D); o.push_back(modrm_rr(dst, src));
    }
    inline void emit_inc_r64(std::vector<uint8_t>& o, uint8_t r) {
        o.push_back(0x48); o.push_back(0xFF); o.push_back(0xC0 | (r & 7));
    }
    inline void emit_dec_r64(std::vector<uint8_t>& o, uint8_t r) {
        o.push_back(0x48); o.push_back(0xFF); o.push_back(0xC8 | (r & 7));
    }

    // 4-byte  REX.W + 83 /grp r64, imm8
    //   grp: 0=ADD, 1=OR, 4=AND, 5=SUB, 6=XOR, 7=CMP
    inline void emit_grp1_r64_imm8(std::vector<uint8_t>& o, uint8_t grp, uint8_t r, uint8_t imm) {
        o.push_back(0x48); o.push_back(0x83);
        o.push_back(0xC0 | ((grp & 7) << 3) | (r & 7));
        o.push_back(imm);
    }

    // 5-byte  MOV r32, imm32
    inline void emit_mov_r32_imm32(std::vector<uint8_t>& o, uint8_t r, uint32_t imm) {
        o.push_back(0xB8 + (r & 7));
        for (int i = 0; i < 4; i++) o.push_back((imm >> (i * 8)) & 0xFF);
    }

    // 5-byte  JMP rel32
    inline void emit_jmp_rel32(std::vector<uint8_t>& o, int32_t off) {
        o.push_back(0xE9);
        uint32_t u = static_cast<uint32_t>(off);
        for (int i = 0; i < 4; i++) o.push_back((u >> (i * 8)) & 0xFF);
    }

    // 2-byte  JMP rel8
    inline void emit_jmp_rel8(std::vector<uint8_t>& o, int8_t off) {
        o.push_back(0xEB); o.push_back(static_cast<uint8_t>(off));
    }

    // 2-byte  Jcc rel8  (cc: 0=O,1=NO,2=B,3=NB,4=Z,5=NZ,6=BE,7=A,8=S,9=NS,A=P,B=NP,C=L,D=GE,E=LE,F=G)
    inline void emit_jcc_rel8(std::vector<uint8_t>& o, uint8_t cc, int8_t off) {
        o.push_back(0x70 | (cc & 0x0F)); o.push_back(static_cast<uint8_t>(off));
    }

    // 7-byte  REX.W MOV r64, sign-ext imm32
    inline void emit_mov_r64_imm32(std::vector<uint8_t>& o, uint8_t r, uint32_t imm) {
        o.push_back(0x48); o.push_back(0xC7); o.push_back(0xC0 | (r & 7));
        for (int i = 0; i < 4; i++) o.push_back((imm >> (i * 8)) & 0xFF);
    }

    // 4-byte  MOV [RSP + disp8], r32   (for fake local variable access)
    inline void emit_mov_rsp_disp8_r32(std::vector<uint8_t>& o, int8_t disp, uint8_t r) {
        // 89 44 24 XX  (SIB byte needed for RSP-based addressing)
        o.push_back(0x89); o.push_back(0x44 | ((r & 7) << 3));
        o.push_back(0x24); o.push_back(static_cast<uint8_t>(disp));
    }

    // 4-byte  MOV r32, [RSP + disp8]
    inline void emit_mov_r32_rsp_disp8(std::vector<uint8_t>& o, uint8_t r, int8_t disp) {
        o.push_back(0x8B); o.push_back(0x44 | ((r & 7) << 3));
        o.push_back(0x24); o.push_back(static_cast<uint8_t>(disp));
    }

    inline void emit_antidisasm_jmp1(std::vector<uint8_t>& o, std::mt19937& rng) {
        o.push_back(0xEB); o.push_back(0x01);
        o.push_back(rand_byte(rng)); // garbage byte (never executed)
    }

    inline void emit_antidisasm_jmp2(std::vector<uint8_t>& o, std::mt19937& rng) {
        o.push_back(0xEB); o.push_back(0x02);
        o.push_back(rand_byte(rng));
        o.push_back(rand_byte(rng));
    }

    inline void emit_opaque_predicate_never(std::vector<uint8_t>& o, std::mt19937& rng, uint8_t dead_bytes) {
        uint8_t r = rand_reg(rng);
        // XOR r32, r32  → r=0, ZF=1
        emit_xor_r32(o, r, r);     // 2 bytes
        // TEST r32, r32  → ZF=1 still
        emit_test_r32(o, r, r);    // 2 bytes
        // JNZ +dead_bytes  → never taken
        emit_jcc_rel8(o, 0x05 /*JNZ*/, static_cast<int8_t>(dead_bytes)); // 2 bytes
        // dead code (never reached)
        for (uint8_t i = 0; i < dead_bytes; i++)
            o.push_back(rand_byte(rng));
    }

    inline void emit_opaque_predicate_always(std::vector<uint8_t>& o, std::mt19937& rng, int8_t jmp_offset) {
        uint8_t r = rand_reg(rng);
        // OR r64, -1 → reg = 0xFFFFFFFFFFFFFFFF, ZF=0
        emit_grp1_r64_imm8(o, 1/*OR*/, r, 0xFF); // 4 bytes
        // JNZ +offset → always taken
        emit_jcc_rel8(o, 0x05/*JNZ*/, jmp_offset); // 2 bytes
    }

    inline void emit_random_1byte(std::vector<uint8_t>& o, std::mt19937& rng) {
        int choice = rng() % 4;
        switch (choice) {
        case 0: emit_nop(o); break;
        case 1: o.push_back(0xF8); break; // CLC
        case 2: o.push_back(0xF9); break; // STC
        case 3: o.push_back(0xFC); break; // CLD
        }
    }

    inline void emit_random_2byte(std::vector<uint8_t>& o, std::mt19937& rng) {
        uint8_t r1 = rand_reg(rng), r2 = rand_reg(rng);
        int choice = rng() % 7;
        switch (choice) {
        case 0: emit_test_r32(o, r1, r2); break;
        case 1: emit_xor_r32(o, r1, r2); break;
        case 2: emit_mov_r32(o, r1, r2); break;
        case 3: emit_cmp_r32(o, r1, r2); break;
        case 4: emit_and_r32(o, r1, r2); break;
        case 5: emit_or_r32(o, r1, r2); break;
        case 6: emit_add_r32(o, r1, r2); break;
        }
    }

    inline void emit_random_3byte(std::vector<uint8_t>& o, std::mt19937& rng) {
        uint8_t r1 = rand_reg(rng), r2 = rand_reg(rng);
        int choice = rng() % 8;
        switch (choice) {
        case 0: emit_mov_r64(o, r1, r2); break;
        case 1: emit_xor_r64(o, r1, r2); break;
        case 2: emit_test_r64(o, r1, r2); break;
        case 3: emit_add_r64(o, r1, r2); break;
        case 4: emit_sub_r64(o, r1, r2); break;
        case 5: emit_cmp_r64(o, r1, r2); break;
        case 6: emit_inc_r64(o, r1); break;
        case 7: emit_dec_r64(o, r1); break;
        }
    }

    inline void emit_random_4byte(std::vector<uint8_t>& o, std::mt19937& rng) {
        uint8_t r = rand_reg(rng);
        uint8_t imm = rand_byte(rng);
        int choice = rng() % 7;
        uint8_t grps[] = { 0, 1, 4, 5, 6, 7 }; // ADD, OR, AND, SUB, XOR, CMP
        switch (choice) {
        case 0: case 1: case 2: case 3: case 4: case 5:
            emit_grp1_r64_imm8(o, grps[choice], r, imm); break;
        case 6:
            emit_mov_rsp_disp8_r32(o, static_cast<int8_t>(0x08 + (rng() % 6) * 8), r); break;
        }
    }

    inline void emit_random_5byte(std::vector<uint8_t>& o, std::mt19937& rng) {
        uint8_t r = rand_reg(rng);
        emit_mov_r32_imm32(o, r, rand_dword(rng));
    }

    inline void emit_random_7byte(std::vector<uint8_t>& o, std::mt19937& rng) {
        uint8_t r = rand_reg(rng);
        emit_mov_r64_imm32(o, r, rand_dword(rng));
    }

    // Emit a random instruction fitting within 'max_bytes' available
    inline void emit_random_instruction(std::vector<uint8_t>& o, size_t max_bytes, std::mt19937& rng) {
        if (max_bytes >= 7 && (rng() % 8 == 0)) { emit_random_7byte(o, rng); return; }
        if (max_bytes >= 5 && (rng() % 6 == 0)) { emit_random_5byte(o, rng); return; }
        if (max_bytes >= 4 && (rng() % 4 == 0)) { emit_random_4byte(o, rng); return; }
        if (max_bytes >= 3 && (rng() % 3 == 0)) { emit_random_3byte(o, rng); return; }
        if (max_bytes >= 2 && (rng() % 2 == 0)) { emit_random_2byte(o, rng); return; }
        emit_random_1byte(o, rng);
    }

    inline std::vector<uint8_t> generate_basic_block(size_t size, std::mt19937& rng) {
        std::vector<uint8_t> block;
        block.reserve(size);
        while (block.size() < size) {
            size_t remaining = size - block.size();
            emit_random_instruction(block, remaining, rng);
        }
        return block;
    }

    inline std::vector<uint8_t> generate_obfuscated_code(size_t total_size, std::mt19937& rng) {
        if (total_size < 10) {
            return generate_basic_block(total_size, rng);
        }

        // Each block needs at least 8 bytes (some content + a 2-byte JMP rel8)
        size_t min_block = 8;
        size_t max_blocks = total_size / min_block;
        if (max_blocks < 2) max_blocks = 2;
        if (max_blocks > 32) max_blocks = 32;
        size_t num_blocks = 2 + (rng() % (max_blocks - 1));

        struct Block {
            size_t layout_pos;
            size_t content_size;
            size_t total_size;
            std::vector<uint8_t> code;
            int exec_next;
        };

        std::vector<Block> blocks(num_blocks);

        std::vector<int> exec_order(num_blocks);
        for (size_t i = 0; i < num_blocks; i++) exec_order[i] = static_cast<int>(i);

        std::vector<int> layout_order(num_blocks);
        for (size_t i = 0; i < num_blocks; i++) layout_order[i] = static_cast<int>(i);
        std::shuffle(layout_order.begin(), layout_order.end(), rng);
        size_t jmp_overhead = (num_blocks - 1) * 5;
        if (jmp_overhead >= total_size) {
            num_blocks = (total_size / 6);
            if (num_blocks < 2) num_blocks = 2;
            blocks.resize(num_blocks);
            exec_order.resize(num_blocks);
            layout_order.resize(num_blocks);
            for (size_t i = 0; i < num_blocks; i++) {
                exec_order[i] = static_cast<int>(i);
                layout_order[i] = static_cast<int>(i);
            }
            std::shuffle(layout_order.begin(), layout_order.end(), rng);
            jmp_overhead = (num_blocks - 1) * 5;
        }

        size_t content_budget = total_size - jmp_overhead;

        std::vector<size_t> content_sizes(num_blocks, 0);
        size_t distributed = 0;
        for (size_t i = 0; i < num_blocks - 1; i++) {
            size_t avg = (content_budget - distributed) / (num_blocks - i);
            size_t min_sz = 1;
            size_t max_sz = min(avg * 2, content_budget - distributed - (num_blocks - i - 1));
            if (max_sz < min_sz) max_sz = min_sz;
            size_t sz = min_sz + (rng() % (max_sz - min_sz + 1));
            content_sizes[i] = sz;
            distributed += sz;
        }
        content_sizes[num_blocks - 1] = content_budget - distributed;

        for (size_t i = 0; i < num_blocks; i++) {
            blocks[i].exec_next = (i + 1 < num_blocks) ? static_cast<int>(i + 1) : -1;
            blocks[i].content_size = content_sizes[i];
            blocks[i].total_size = content_sizes[i] + ((i + 1 < num_blocks) ? 5 : 0);
        }

        size_t pos = 0;
        for (size_t i = 0; i < num_blocks; i++) {
            int blk_idx = layout_order[i];
            blocks[blk_idx].layout_pos = pos;
            pos += blocks[blk_idx].total_size;
        }

        for (size_t i = 0; i < num_blocks; i++) {
            auto& blk = blocks[i];
            blk.code.reserve(blk.total_size);

            size_t content_remaining = blk.content_size;

            while (content_remaining > 0) {
                if (content_remaining >= 6 && (rng() % 5 == 0)) {
                    uint8_t r = rand_reg(rng);
                    emit_xor_r32(blk.code, r, r);
                    emit_test_r32(blk.code, r, r);
                    uint8_t fake_offset = static_cast<uint8_t>(1 + rng() % 10);
                    emit_jcc_rel8(blk.code, 0x05, static_cast<int8_t>(fake_offset));
                    content_remaining -= 6;
                }
                else if (content_remaining >= 3 && (rng() % 6 == 0)) {
                    emit_antidisasm_jmp1(blk.code, rng);
                    content_remaining -= 3;
                }
                else if (content_remaining >= 4 && (rng() % 7 == 0)) {
                    emit_antidisasm_jmp2(blk.code, rng);
                    content_remaining -= 4;
                }
                else if (content_remaining >= 2 && (rng() % 5 == 0)) {
                    uint8_t r = rand_reg(rng);
                    if (content_remaining >= 2) {
                        emit_push(blk.code, r);
                        emit_pop(blk.code, r);
                        content_remaining -= 2;
                    }
                }
                else {
                    size_t before = blk.code.size();
                    emit_random_instruction(blk.code, content_remaining, rng);
                    content_remaining -= (blk.code.size() - before);
                }
            }

            if (blk.exec_next >= 0) {
                int next_blk = blk.exec_next;
                size_t jmp_end = blk.layout_pos + blk.code.size() + 5; // +5 for JMP rel32
                size_t target = blocks[next_blk].layout_pos;
                int32_t disp = static_cast<int32_t>(target) - static_cast<int32_t>(jmp_end);
                emit_jmp_rel32(blk.code, disp);
            }
        }

        std::vector<uint8_t> result(total_size, 0xCC); // fill with INT3 as default
        for (size_t i = 0; i < num_blocks; i++) {
            auto& blk = blocks[i];
            if (blk.layout_pos + blk.code.size() <= total_size) {
                std::memcpy(result.data() + blk.layout_pos, blk.code.data(), blk.code.size());
            }
        }

        if (blocks[0].layout_pos != 0) {

        }

        return result;
    }

    // =========================================================================
    // Anti-disassembly junk for dead code regions (after unconditional JMPs).
    // These bytes are NEVER executed. They exist solely to break x64dbg's
    // linear-sweep disassembler by emitting:
    //   - Partial multi-byte opcode escapes (0x0F, 0x0F38, 0x0F3A)
    //   - Bogus VEX/EVEX prefixes (0xC4, 0xC5, 0x62)
    //   - LOCK prefix before invalid opcodes (triggers #UD, confuses disasm)
    //   - Fake REX.W prefixes that eat the next "instruction"
    //   - Overlapping jmp-into-middle patterns
    //   - Invalid ModR/M + SIB combos that consume unpredictable lengths
    // =========================================================================

    // Emit a single anti-disasm pattern, returns bytes written
    inline size_t emit_antidisasm_pattern(uint8_t* buf, size_t space, std::mt19937& rng) {
        if (space == 0) return 0;

        int pattern = rng() % 18;

        switch (pattern) {

        case 0: // Fake VEX 3-byte prefix (0xC4) + garbage → disasm tries to parse VEX fields
            if (space >= 5) {
                buf[0] = 0xC4;
                buf[1] = rand_byte(rng) | 0xC0; // R/X bits set to look like reg-reg
                buf[2] = rand_byte(rng);
                buf[3] = 0x0F + (rng() % 0xF0); // "opcode" that doesn't exist in VEX map
                buf[4] = rand_byte(rng);
                return 5;
            }
            break;

        case 1: // Fake VEX 2-byte prefix (0xC5) + garbage
            if (space >= 4) {
                buf[0] = 0xC5;
                buf[1] = rand_byte(rng) | 0x80; // high bit set
                buf[2] = rand_byte(rng);
                buf[3] = modrm_rr(rand_reg(rng), rand_reg(rng));
                return 4;
            }
            break;

        case 2: // EVEX prefix (0x62) + 3 payload bytes → x64dbg tries to decode EVEX
            if (space >= 6) {
                buf[0] = 0x62;
                buf[1] = rand_byte(rng) | 0xF0; // P0: set top bits
                buf[2] = rand_byte(rng) | 0x04; // P1: W bit and some vvvv
                buf[3] = rand_byte(rng);         // P2
                buf[4] = rand_byte(rng);         // "opcode"
                buf[5] = rand_byte(rng);         // "modrm"
                return 6;
            }
            break;

        case 3: // 0x0F escape + invalid second byte → disasm shows "???" or misparses length
            if (space >= 3) {
                buf[0] = 0x0F;
                // Pick bytes that aren't valid 0x0F XX opcodes
                uint8_t invalid_2nd[] = { 0x04, 0x0A, 0x0C, 0x24, 0x25, 0x26, 0x27, 0x36, 0x37, 0x39, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x07, 0x0E, 0xA6, 0xA7 };
                buf[1] = invalid_2nd[rng() % sizeof(invalid_2nd)];
                buf[2] = rand_byte(rng);
                return 3;
            }
            break;

        case 4: // 0x0F 0x3A (3-byte opcode map) + invalid opcode + imm8
            if (space >= 5) {
                buf[0] = 0x0F;
                buf[1] = 0x3A;
                buf[2] = 0xCC + (rng() % 0x30); // invalid opcode in 0F3A map
                buf[3] = modrm_rr(rng() % 8, rng() % 8);
                buf[4] = rand_byte(rng); // "imm8"
                return 5;
            }
            break;

        case 5: // 0x0F 0x38 + invalid opcode
            if (space >= 4) {
                buf[0] = 0x0F;
                buf[1] = 0x38;
                buf[2] = 0xF2 + (rng() % 0x0D); // high end of 0F38 map, mostly undefined
                buf[3] = modrm_rr(rng() % 8, rng() % 8);
                return 4;
            }
            break;

        case 6: // LOCK + non-lockable opcode → #UD on execution, disasm shows "lock ???"
            if (space >= 4) {
                buf[0] = 0xF0; // LOCK
                buf[1] = 0x48; // REX.W
                buf[2] = 0x8D; // LEA (not lockable!)
                buf[3] = modrm_rr(rand_reg(rng), rand_reg(rng));
                return 4;
            }
            break;

        case 7: // REX.W + 0xFF with /7 (invalid extension) → unknown instruction
            if (space >= 3) {
                buf[0] = 0x48; // REX.W
                buf[1] = 0xFF;
                buf[2] = 0xF8 | (rng() % 8); // mod=11, reg=7 (invalid for FF group)
                return 3;
            }
            break;

        case 8: // Fake CALL [RIP+disp32] with huge displacement → disasm shows call to garbage
            if (space >= 6) {
                buf[0] = 0xFF;
                buf[1] = 0x15; // CALL [RIP+disp32]
                buf[2] = rand_byte(rng);
                buf[3] = rand_byte(rng);
                buf[4] = rand_byte(rng);
                buf[5] = rand_byte(rng);
                return 6;
            }
            break;

        case 9: // Overlapping: JMP +1 into middle of a multi-byte "instruction"
            if (space >= 6) {
                buf[0] = 0xEB; buf[1] = 0x01;  // JMP +1
                buf[2] = 0x0F;                   // eaten by JMP, but disasm tries to decode 0F XX
                buf[3] = 0x0F;                   // actual landing: this starts new "0F XX"
                buf[4] = rand_byte(rng);          // garbage second byte
                buf[5] = rand_byte(rng);
                return 6;
            }
            break;

        case 10: // Multiple redundant REX prefixes → only last is used, disasm may misparse
            if (space >= 5) {
                buf[0] = 0x40 | (rng() % 16); // REX
                buf[1] = 0x40 | (rng() % 16); // REX (overrides first)
                buf[2] = 0x40 | (rng() % 16); // REX (overrides second)
                buf[3] = 0x0F;                 // two-byte escape
                buf[4] = rand_byte(rng);       // garbage
                return 5;
            }
            break;

        case 11: // Segment override prefixes chained + garbage (x64 mostly ignores these)
            if (space >= 5) {
                uint8_t segs[] = { 0x26, 0x2E, 0x36, 0x3E, 0x64, 0x65 };
                buf[0] = segs[rng() % 6];
                buf[1] = segs[rng() % 6];
                buf[2] = 0x0F;
                buf[3] = rand_byte(rng);
                buf[4] = rand_byte(rng);
                return 5;
            }
            break;

        case 12: // 0xF3 0x0F (rep prefix + escape) with invalid opcode → fake SSE
            if (space >= 4) {
                buf[0] = 0xF3;
                buf[1] = 0x0F;
                uint8_t bad_sse[] = { 0x04, 0x0A, 0x24, 0x25, 0x26, 0x27, 0x36, 0x78, 0x79 };
                buf[2] = bad_sse[rng() % sizeof(bad_sse)];
                buf[3] = modrm_rr(rng() % 8, rng() % 8);
                return 4;
            }
            break;

        case 13: // 0xF2 0x0F (repne prefix + escape) with invalid opcode → fake SSE2
            if (space >= 4) {
                buf[0] = 0xF2;
                buf[1] = 0x0F;
                buf[2] = 0x04 + (rng() % 8); // mostly undefined in F2 0F map
                buf[3] = modrm_rr(rng() % 8, rng() % 8);
                return 4;
            }
            break;

        case 14: // Incomplete RIP-relative MOV with SIB that eats extra bytes
            if (space >= 7) {
                buf[0] = 0x48;                     // REX.W
                buf[1] = 0x8B;                     // MOV r64, r/m64
                buf[2] = 0x04 | ((rng() % 8) << 3); // ModR/M: mod=00, rm=100 (SIB follows)
                buf[3] = 0x25;                     // SIB: base=101 (disp32, no base), index=100 (none)
                buf[4] = rand_byte(rng);           // disp32 byte 0
                buf[5] = rand_byte(rng);           // disp32 byte 1
                buf[6] = rand_byte(rng);           // disp32 byte 2 → incomplete! disasm expects byte 3
                return 7;
            }
            break;

        case 15: // UD2 (0x0F 0x0B) followed by garbage that looks like instruction stream
            if (space >= 4) {
                buf[0] = 0x0F;
                buf[1] = 0x0B; // UD2 - guaranteed undefined instruction
                buf[2] = 0x48 | (rng() % 8); // looks like REX prefix
                buf[3] = rand_byte(rng);
                return 4;
            }
            break;

        case 16: // Fake JMP rel32 pointing into garbage → disasm follows and desyncs
            if (space >= 7) {
                buf[0] = 0xE9; // JMP rel32
                // Small forward offset that lands in garbage
                buf[1] = 0x00;
                buf[2] = 0x01; // ~256 bytes forward (beyond this region)
                buf[3] = 0x00;
                buf[4] = 0x00;
                buf[5] = 0x0F; // disasm tries these as next insn
                buf[6] = rand_byte(rng);
                return 7;
            }
            break;

        case 17: // INT 0x29 + REX.W + partial opcode → disasm trips on INT imm8 length
            if (space >= 4) {
                buf[0] = 0xCD; // INT imm8
                buf[1] = 0x48; // looks like REX.W to disasm scanning linearly
                buf[2] = 0xFF;
                buf[3] = 0x25; // JMP [rip+disp32]... but only 0 disp bytes follow
                return 4;
            }
            break;
        }

        // Fallback: single garbage byte that isn't a valid 1-byte opcode
        uint8_t bad_singles[] = { 0x06, 0x07, 0x0E, 0x16, 0x17, 0x1E, 0x1F, 0x27, 0x2F, 0x37, 0x3F, 0xD6, 0xF1 };
        buf[0] = bad_singles[rng() % sizeof(bad_singles)];
        return 1;
    }

    // Fill a buffer of `size` bytes with anti-disassembly junk.
    // Designed to be used in dead code regions after unconditional JMPs
    // to maximally confuse x64dbg's linear sweep disassembler.
    inline void generate_antidisasm_junk(uint8_t* buf, size_t size, std::mt19937& rng) {
        size_t pos = 0;
        while (pos < size) {
            size_t remaining = size - pos;
            size_t written = emit_antidisasm_pattern(buf + pos, remaining, rng);
            if (written == 0) {
                // Should never happen, but safety: fill rest with invalid bytes
                buf[pos] = 0x06; // PUSH ES - invalid in x64 long mode
                written = 1;
            }
            pos += written;
        }
    }

} // namespace opcode

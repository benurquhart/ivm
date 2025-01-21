#pragma once
#include <iostream>
#include <vector>
#include <cstring>
#include <cstdint>

namespace ivm {
	typedef uintptr_t ivm_instr;
	typedef uintptr_t ivm_rgstr;

	typedef uint8_t ivm_opcode;

	constexpr auto ivm_seed = 5432167890123456789;

	namespace {
		struct ivm_default {

			uint8_t _padd[8];
		};

		constexpr auto IVM_MOV = 1;
		constexpr auto IVM_ADD = 2;
		constexpr auto IVM_SUB = 3;
		constexpr auto IVM_AND = 4;
		constexpr auto IVM_XOR = 5;
		constexpr auto IVM_LEA = 6;
		constexpr auto IVM_CMP = 7;
		constexpr auto IVM_JNE = 8;
		constexpr auto IVM_CNA = 9;
		constexpr auto IVM_RET = 10;

		constexpr auto IVM_DEREF_SRC = 1;
		constexpr auto IVM_DEREF_DST = 2;

		constexpr auto IVM_IMMV = 1;

		//

		template <typename T = ivm_default>
		constexpr auto calc_deref() -> int {

			return std::is_pointer_v<T> ? IVM_DEREF_DST : (!std::is_same_v<T, ivm_default> ? IVM_DEREF_SRC : 0);
		}

		template <typename T = ivm_default>
		constexpr auto calc_size() -> int {

			using T_SIZE = typename std::remove_pointer<T>::type;

			return (sizeof(T_SIZE) > 8 ? 7 : sizeof(T_SIZE) - 1);
		}

		template <typename T = ivm_default>
		constexpr auto build_instr(ivm_rgstr rgstr_1, ivm_rgstr rgstr_2, ivm_opcode opcode) -> ivm_instr {

			constexpr auto deref = calc_deref<T>();
			constexpr auto size = calc_size<T>();

			auto immv_flag = rgstr_2 == 0 ? (IVM_IMMV << 7) : 0;

			rgstr_1--; if (rgstr_2) rgstr_2--;

			return (unsigned short)((((deref << 6) | (rgstr_2 << 3) | rgstr_1) << 8) | ((immv_flag) | (size << 4) | opcode)) ^ ivm_seed;

			// 04: [opcode | size | immv]
			// 00: [deref  | reg1 | reg2]
		}
		
		constexpr auto get_opcode(uint16_t instr) -> ivm_opcode {

			return instr & 0xF;
		}

		constexpr auto get_immv(uint16_t instr) -> uint16_t {

			return (instr >> 7) & IVM_IMMV;
		}

		constexpr auto get_size(uint16_t instr) -> uint16_t {

			return ((instr >> 4) & 0x7) + 1;
		}

		constexpr auto get_deref(uint8_t oprnd) -> uint16_t {

			return (oprnd >> 6) & 0x3;
		}

		constexpr auto get_rgstr_1(uint8_t oprnd) -> uint16_t {

			return oprnd & 0x7;
		}

		constexpr auto get_rgstr_2(uint8_t oprnd) -> uint16_t {

			return (oprnd >> 3) & 0x7;
		}
	}

	constexpr auto R0 = ivm_rgstr(1);
	constexpr auto R1 = ivm_rgstr(2);
	constexpr auto R2 = ivm_rgstr(3);
	constexpr auto R3 = ivm_rgstr(4);
	constexpr auto R4 = ivm_rgstr(5);
	constexpr auto R5 = ivm_rgstr(6);

	#define DEFINE_INSTR(name, opcode)					\
	template <typename T = ivm_default>					\
	constexpr auto name(ivm_rgstr rgstr_1 = 0, ivm_rgstr rgstr_2 = 0)	\
		-> ivm_instr {							\
		return build_instr<T>(rgstr_1, rgstr_2, opcode);		\
	}

	DEFINE_INSTR(MOV, IVM_MOV)
	DEFINE_INSTR(ADD, IVM_ADD)
	DEFINE_INSTR(SUB, IVM_SUB)
	DEFINE_INSTR(XOR, IVM_XOR)
	DEFINE_INSTR(LEA, IVM_LEA)
	DEFINE_INSTR(CMP, IVM_CMP)
	DEFINE_INSTR(JNE, IVM_JNE)
	DEFINE_INSTR(CNA, IVM_CNA)
	DEFINE_INSTR(RET, IVM_RET)

	#undef DEFINE_INSTR

	//

	auto ivm(const std::vector<ivm_instr>& prgm, bool dbg = true) -> void {

		auto prgm_counter = 0, zf = 0;

		ivm_rgstr rgstr[6] = { 0 };

		while (prgm_counter < prgm.size()) {

			const auto prgm_instr = (unsigned short)(prgm[prgm_counter]) ^ ivm_seed;

			const auto instr = (unsigned char)((prgm_instr & 0xFF));
			const auto oprnd = (unsigned char)((prgm_instr >> 8) & 0xFF);
			
			const auto opcode = get_opcode(instr);

			const auto immv = get_immv(instr);
			const auto size = get_size(instr);

			const auto deref = get_deref(oprnd);

			const auto rgstr_1 = get_rgstr_1(oprnd);
			const auto rgstr_2 = get_rgstr_2(oprnd);

			auto src = immv ? &prgm[prgm_counter + 1] : &rgstr[rgstr_2];

			auto dst = &rgstr[rgstr_1];

			if (deref == IVM_DEREF_SRC) {

				src = *(uintptr_t**)src;
			}
			else if (deref == IVM_DEREF_DST) {

				dst = *(uintptr_t**)dst;
			}

			switch (opcode) {

				case IVM_MOV: {
					memcpy(dst, src, size);
					break;
				}
				case IVM_ADD: {
					*dst += *src;
					break;
				}
				case IVM_SUB: {
					*dst -= *src;
					break;
				}
				case IVM_AND: {
					*dst &= *src;
					break;
				}
				case IVM_XOR: {
					*dst ^= *src;
					break;
				}
				case IVM_LEA: {
					*dst = (uintptr_t)src;
					break;
				}
				case IVM_CMP: {
					zf = ((*dst - *src) == 0);
					break;
				}
				case IVM_JNE: {
					if (zf == 0) {
						prgm_counter = *(int*)src;
						continue;
					}
					break;
				}
				case IVM_CNA: {
					rgstr[0] = ((ivm_rgstr(*)(...))*src)(
						rgstr[1],
						rgstr[2],
						rgstr[3],
						rgstr[4],
						rgstr[5]
					);
					break;
				}
				case IVM_RET: {
					prgm_counter = prgm.size();
					break;
				}
			}

			prgm_counter += immv ? 2 : 1;
		};

		if (dbg) {

			std::cout << "R0: 0x" << std::hex << rgstr[0] << std::endl;
			std::cout << "R1: 0x" << std::hex << rgstr[1] << std::endl;
			std::cout << "R2: 0x" << std::hex << rgstr[2] << std::endl;
			std::cout << "R3: 0x" << std::hex << rgstr[3] << std::endl;
			std::cout << "R4: 0x" << std::hex << rgstr[4] << std::endl;
			std::cout << "R5: 0x" << std::hex << rgstr[5] << std::endl;
		}
	}
}
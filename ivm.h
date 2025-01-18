#pragma once
#include <iostream>
#include <vector>

namespace ivm {

	typedef uintptr_t ivm_instr;
	typedef uintptr_t ivm_rgstr;

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

		constexpr auto IVM_DEREF_NONE = 0;
		constexpr auto IVM_DEREF_SRC = 1;
		constexpr auto IVM_DEREF_DST = 2;

		constexpr auto IVM_IMMV = 1;

		//

		template <typename T = ivm_default>
		constexpr auto calc_deref() -> int {

			return std::is_pointer_v<T> ? IVM_DEREF_DST : (!std::is_same_v<T, ivm_default> ? IVM_DEREF_SRC : IVM_DEREF_NONE);
		}

		template <typename T = ivm_default>
		constexpr auto calc_size() -> int {

			return (sizeof(T) > 8 ? 7 : sizeof(T) - 1);
		}

		template <typename T = ivm_default>
		constexpr auto build_instr(ivm_rgstr rgstr1, ivm_rgstr rgstr2, int opcode) -> ivm_instr {

			constexpr auto deref = calc_deref<T>();
			constexpr auto size = calc_size<T>();

			auto immv_flag = rgstr2 == 0 ? (IVM_IMMV << 7) : 0;

			rgstr1--;
			if (rgstr2) {
				rgstr2--;
			}

			return (unsigned short)((((deref << 6) | (rgstr2 << 3) | rgstr1) << 8) | ((immv_flag) | (size << 4) | opcode)) ^ ivm_seed;

			// 04: [opcode | size | immv]
			// 00: [deref  | reg1 | reg2]
		}
	}

	constexpr auto R0 = ivm_rgstr(1);
	constexpr auto R1 = ivm_rgstr(2);
	constexpr auto R2 = ivm_rgstr(3);
	constexpr auto R3 = ivm_rgstr(4);
	constexpr auto R4 = ivm_rgstr(5);
	constexpr auto R5 = ivm_rgstr(6);

	#define DEFINE_INSTR(name, opcode)								\
	template <typename T = ivm_default>								\
	constexpr auto name(ivm_rgstr rgstr1, ivm_rgstr rgstr2 = 0)		\
		-> uint16_t {												\
		return build_instr<T>(rgstr1, rgstr2, opcode);				\
	}

	DEFINE_INSTR(MOV, IVM_MOV)
	DEFINE_INSTR(ADD, IVM_ADD)
	DEFINE_INSTR(SUB, IVM_SUB)
	DEFINE_INSTR(XOR, IVM_XOR)
	DEFINE_INSTR(LEA, IVM_LEA)
	DEFINE_INSTR(CMP, IVM_CMP)
	DEFINE_INSTR(JNE, IVM_CMP, 0)
	DEFINE_INSTR(CNA, IVM_CMP, 0)

	#undef DEFINE_INSTR

	#define GET_IMMV(instr) (((instr) >> 7) & IVM_IMMV)
	#define GET_SIZE(instr) ((((instr) >> 4) & 0x7) + 1)
	#define GET_DEREF(oprnd) (((oprnd) >> 6) & 0x3)
	#define GET_RGSTR1(oprnd) ((oprnd) & 0x7)
	#define GET_RGSTR2(oprnd) (((oprnd) >> 3) & 0x7)
		
	//

	auto ivm(const std::vector<ivm_instr>& prgm, bool dbg = true) -> void {

		auto prgm_counter = 0, zf = 0;

		ivm_rgstr rgstr[6] = { 0 };

		while (prgm_counter < prgm.size()) {

			const auto prgm_instr = ((unsigned short)(prgm[prgm_counter]) ^ ivm_seed);

			const auto instr = (unsigned char)(prgm_instr & 0xFF);
			const auto oprnd = (unsigned char)((prgm_instr >> 8) & 0xFF);

			const auto immv = GET_IMMV(instr);
			const auto size = GET_SIZE(instr);

			const auto deref = GET_DEREF(oprnd);

			const auto rgstr1 = GET_RGSTR1(oprnd);
			const auto rgstr2 = GET_RGSTR2(oprnd);

			auto src = immv ? &prgm[prgm_counter + 1] : &rgstr[rgstr2];

			auto dst = &rgstr[rgstr1];

			switch (instr & 0xF) {

			case IVM_MOV: {
				memcpy(deref == 1 ? dst : (deref == 2 ? *(void**)dst : dst), deref == 1 ? *(void**)src : src, size);
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
					prgm_counter = *src;
					continue;
				}
				break;
			}
			case IVM_CNA: {
				rgstr[0] = ((ivm_rgstr(*)(...)) * src)(rgstr[1], rgstr[2], rgstr[3], rgstr[4], rgstr[5]);
				break;
			}
			}

			prgm_counter += immv ? 2 : 1;
		};

		if (dbg) {

			printf(
				"R0: 0x%IX\n"
				"R1: 0x%IX\n"
				"R2: 0x%IX\n"
				"R3: 0x%IX\n"
				"R4: 0x%IX\n",

				rgstr[0],
				rgstr[1],
				rgstr[2],
				rgstr[3],
				rgstr[4],
				rgstr[5]
			);
		}
	}
}
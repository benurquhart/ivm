#pragma once
#include <iostream>
#include <vector>
#include <cstring>
#include <cstdint>

namespace ivm {

	constexpr auto IVM_SEED = 5432167890123456789;
	constexpr auto IVM_STACK_SIZE = 1024;

	namespace internal {

		typedef uintptr_t ivm_instr_t;
		typedef uintptr_t ivm_rgstr_t;
		typedef uint8_t ivm_opcode_t;
		typedef uint8_t ivm_stack_t;

		typedef struct _ivm_default_t {

			uint8_t _padd[8];

		} ivm_default_t, *pivm_default_t;

		constexpr auto IVM_MOV = 1;
		constexpr auto IVM_ADD = 2;
		constexpr auto IVM_SUB = 3;
		constexpr auto IVM_AND = 4;
		constexpr auto IVM_XOR = 5;
		constexpr auto IVM_LEA = 6;
		constexpr auto IVM_CMP = 7;
		constexpr auto IVM_JNE = 8;
		constexpr auto IVM_CNA = 9;
		constexpr auto IVM_PUSH = 10;
		constexpr auto IVM_POP = 11;
		constexpr auto IVM_RET = 12;

		constexpr auto IVM_DEREF_SRC = 1;
		constexpr auto IVM_DEREF_DST = 2;

		constexpr auto IVM_IMMV = 1;

		//

		template <typename T = ivm_default_t>
		constexpr auto calc_deref() -> const std::size_t {

			return std::is_pointer_v<T> ? IVM_DEREF_DST : (!std::is_same_v<T, ivm_default_t> ? IVM_DEREF_SRC : 0);
		}

		template <typename T = ivm_default_t>
		constexpr auto calc_size() -> const std::size_t {

			using T_SIZE = typename std::remove_pointer<T>::type;

			return (sizeof(T_SIZE) > 8 ? 7 : sizeof(T_SIZE) - 1);
		}

		template <typename T = ivm_default_t>
		constexpr auto build_instr(ivm_rgstr_t rgstr_1, ivm_rgstr_t rgstr_2, ivm_opcode_t opcode) -> const ivm_instr_t {

			constexpr auto deref = calc_deref<T>();
			constexpr auto size = calc_size<T>();

			const auto immv_flag = rgstr_2 == 0 ? (IVM_IMMV << 7) : 0;

			rgstr_1--; if (rgstr_2) rgstr_2--;

			std::uint16_t instr = 0;

			instr |= (deref << 6);
			instr |= (rgstr_2 << 3);
			instr |= rgstr_1;
			instr <<= 8;

			instr |= immv_flag;
			instr |= (size << 4);
			instr |= opcode;

			return instr ^ IVM_SEED;

			// 04: [opcode | size | immv]
			// 00: [deref  | reg1 | reg2]
		}

		constexpr auto get_opcode(std::uint16_t instr) -> const ivm_opcode_t {

			return instr & 0xF;
		}

		constexpr auto get_immv(std::uint16_t instr) -> const std::uint16_t {

			return (instr >> 7) & IVM_IMMV;
		}

		constexpr auto get_size(std::uint16_t instr) -> const std::uint16_t {

			return ((instr >> 4) & 0x7) + 1;
		}

		constexpr auto get_deref(std::uint8_t oprnd) -> const std::uint16_t {

			return (oprnd >> 6) & 0x3;
		}

		constexpr auto get_rgstr_1(std::uint8_t oprnd) -> const std::uint16_t {

			return oprnd & 0x7;
		}

		constexpr auto get_rgstr_2(std::uint8_t oprnd) -> const std::uint16_t {

			return (oprnd >> 3) & 0x7;
		}
	}

	constexpr auto R0 = internal::ivm_rgstr_t(1);
	constexpr auto R1 = internal::ivm_rgstr_t(2);
	constexpr auto R2 = internal::ivm_rgstr_t(3);
	constexpr auto R3 = internal::ivm_rgstr_t(4);
	constexpr auto R4 = internal::ivm_rgstr_t(5);
	constexpr auto R5 = internal::ivm_rgstr_t(6);
	constexpr auto R6 = internal::ivm_rgstr_t(7);
	constexpr auto SP = internal::ivm_rgstr_t(8);

	#define DEFINE_INSTR(name, opcode)								\
	template <typename T = internal::ivm_default_t>							\
	constexpr auto name(internal::ivm_rgstr_t rgstr_1 = 0, internal::ivm_rgstr_t rgstr_2 = 0)	\
		-> internal::ivm_instr_t {								\
													\
		return internal::build_instr<T>(rgstr_1, rgstr_2, opcode);				\
	}

	DEFINE_INSTR(MOV, internal::IVM_MOV)
	DEFINE_INSTR(ADD, internal::IVM_ADD)
	DEFINE_INSTR(SUB, internal::IVM_SUB)
	DEFINE_INSTR(XOR, internal::IVM_XOR)
	DEFINE_INSTR(LEA, internal::IVM_LEA)
	DEFINE_INSTR(CMP, internal::IVM_CMP)
	DEFINE_INSTR(JNE, internal::IVM_JNE)
	DEFINE_INSTR(CNA, internal::IVM_CNA)
	DEFINE_INSTR(RET, internal::IVM_RET)

	#undef DEFINE_INSTR

	template <typename T = internal::ivm_default_t>
	constexpr auto PUSH(internal::ivm_rgstr_t rgstr_1 = 0) -> const internal::ivm_instr_t {

		return internal::build_instr<T>(rgstr_1, 1, internal::IVM_PUSH);
	}

	template <typename T = internal::ivm_default_t>
	constexpr auto POP(internal::ivm_rgstr_t rgstr_1 = 0) -> const internal::ivm_instr_t {

		return internal::build_instr<T>(rgstr_1, 1, internal::IVM_POP);
	}

	//

	auto ivm(const std::vector<internal::ivm_instr_t>& prgm, bool dbg = true) -> void {

		auto prgm_counter = 0, zf = 0;

		internal::ivm_rgstr_t rgstr[8] = { 0 };

		internal::ivm_stack_t stack[IVM_STACK_SIZE] = { 0 };

		rgstr[7] = (reinterpret_cast<std::uintptr_t>(&stack[0])) + IVM_STACK_SIZE;

		while (prgm_counter < prgm.size()) {

			const auto prgm_instr = static_cast<std::uint16_t>(prgm[prgm_counter]) ^ IVM_SEED;

			const auto instr = static_cast<std::uint8_t>((prgm_instr & 0xFF));
			const auto oprnd = static_cast<std::uint8_t>((prgm_instr >> 8) & 0xFF);

			const auto opcode = internal::get_opcode(instr);

			const auto immv = internal::get_immv(instr);
			const auto size = internal::get_size(instr);

			const auto deref = internal::get_deref(oprnd);

			const auto rgstr_1 = internal::get_rgstr_1(oprnd);
			const auto rgstr_2 = internal::get_rgstr_2(oprnd);

			auto src = immv ? &prgm[prgm_counter + 1] : &rgstr[rgstr_2];

			auto dst = &rgstr[rgstr_1];

			if (deref == internal::IVM_DEREF_SRC) {

				src = *(std::uintptr_t**)src;
			}
			else if (deref == internal::IVM_DEREF_DST) {

				dst = *(std::uintptr_t**)dst;
			}

			switch (opcode) {

			case internal::IVM_MOV: {

				memcpy(dst, src, size);

				break;
			}
			case internal::IVM_ADD: {

				*dst += *src;

				break;
			}
			case internal::IVM_SUB: {

				*dst -= *src;

				break;
			}
			case internal::IVM_AND: {

				*dst &= *src;

				break;
			}
			case internal::IVM_XOR: {

				*dst ^= *src;

				break;
			}
			case internal::IVM_LEA: {

				*dst = reinterpret_cast<std::uintptr_t>(src);

				break;
			}
			case internal::IVM_CMP: {

				zf = ((*dst - *src) == 0);

				break;
			}
			case internal::IVM_JNE: {

				if (zf == 0) {

					prgm_counter = *(int*)src;

					continue;
				}

				break;
			}
			case internal::IVM_CNA: {

				rgstr[0] = ((internal::ivm_rgstr_t(*)(...)) * src)(
					rgstr[1],
					rgstr[2],
					rgstr[3],
					rgstr[4],
					rgstr[5],
					rgstr[6]
				);

				break;
			}
			case internal::IVM_PUSH: {

				memcpy(reinterpret_cast<void*>(rgstr[7] -= sizeof(internal::ivm_rgstr_t)), dst, sizeof(internal::ivm_rgstr_t));

				break;
			}
			case internal::IVM_POP: {
				
				memcpy(dst, reinterpret_cast<void*>(rgstr[7]), sizeof(internal::ivm_rgstr_t));

				rgstr[7] += sizeof(internal::ivm_rgstr_t);

				break;
			}
			case internal::IVM_RET: {

				prgm_counter = static_cast<int>(prgm.size());

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
			std::cout << "R6: 0x" << std::hex << rgstr[6] << std::endl;
			std::cout << "SP: 0x" << std::hex << rgstr[7] << std::endl;
		}
	}
}
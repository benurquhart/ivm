#pragma once

#include <iostream>
#include <vector>
#include <cstring>
#include <cstdint>

namespace ivm {

	constexpr std::uint64_t IVM_SEED = 5432167890123456789;

	constexpr std::size_t IVM_STACK_SIZE = 1024;

	namespace internal {

		typedef std::uintptr_t ivm_instr_t;
		typedef std::uintptr_t ivm_rgstr_t;
		typedef std::uintptr_t ivm_value_t;

		typedef std::uint8_t ivm_opcode_t;
		typedef std::uint8_t ivm_stack_t;

		typedef struct _ivm_default_t {

			std::uint8_t _padd[8];

		} ivm_default_t, *pivm_default_t;

		constexpr auto IVM_MOV = 1;
		constexpr auto IVM_ADD = 2;
		constexpr auto IVM_SUB = 3;
		constexpr auto IVM_AND = 4;
		constexpr auto IVM_XOR = 5;
		constexpr auto IVM_LEA = 6;
		constexpr auto IVM_CMP = 7;
		constexpr auto IVM_JNE = 8;
		constexpr auto IVM_JMP = 9;
		constexpr auto IVM_CALL = 10;
		constexpr auto IVM_PUSH = 11;
		constexpr auto IVM_POP = 12;
		constexpr auto IVM_RET = 13;

		constexpr auto IVM_DREF_SRC = 1;
		constexpr auto IVM_DREF_DST = 2;
		constexpr auto IVM_DREF_CPY = IVM_DREF_SRC | IVM_DREF_DST;

		constexpr auto IVM_IMMV = 1;

		//

		template <typename T = ivm_default_t, typename TT = void>
		constexpr auto calc_dref() -> const std::uint8_t {

			if (std::is_pointer_v<T> && std::is_pointer_v<TT>) {

				return IVM_DREF_CPY;
			}
			else if (std::is_pointer_v<T> && std::is_same_v<TT, void>) {

				return IVM_DREF_SRC;
			}
			else if (std::is_pointer_v<T>) {

				return IVM_DREF_DST;
			}
			else if (std::is_pointer_v<TT>) {

				return IVM_DREF_SRC;
			}

			return 0;
		}

		template <typename T = ivm_default_t>
		constexpr auto calc_size() -> std::size_t {

			return std::min(sizeof(std::remove_pointer_t<T>), static_cast<std::size_t>(8));
		}

		template <typename T = ivm_default_t, typename TT = void>
		constexpr auto build_instr(ivm_rgstr_t rgstr_1, ivm_rgstr_t rgstr_2, const ivm_opcode_t opcode) -> const ivm_instr_t {

			constexpr auto dref = calc_dref<T, TT>();
			constexpr auto size = calc_size<T>();

			const auto immv = (rgstr_2 == 0) ? 1 : 0;

			rgstr_1--; if (rgstr_2) rgstr_2--;

			ivm_instr_t instr = 0;

			instr |= static_cast<ivm_instr_t>(opcode);
			
			instr |= static_cast<ivm_instr_t>((rgstr_2 & 0xF)) << 8;
			instr |= static_cast<ivm_instr_t>((rgstr_1 & 0xF)) << 12;

			instr |= static_cast<ivm_instr_t>((dref & 0x3)) << 16;
			instr |= static_cast<ivm_instr_t>((immv & 0x1)) << 18;
			instr |= static_cast<ivm_instr_t>((size & 0x1F)) << 19;

			return instr ^ IVM_SEED;
		}

		constexpr auto unpack_opcode(const ivm_instr_t instr) -> const ivm_opcode_t {

			return static_cast<ivm_opcode_t>(instr & 0xFF);
		}

		constexpr auto unpack_immv(const ivm_instr_t instr) -> const std::uint8_t {

			return static_cast<std::uint8_t>((instr >> 18) & 0x1);
		}

		constexpr auto unpack_size(const ivm_instr_t instr) -> const std::uint8_t {

			return static_cast<std::uint8_t>((instr >> 19) & 0x1F);
		}

		constexpr auto unpack_dref(const ivm_instr_t instr) -> const std::uint8_t {

			return static_cast<std::uint8_t>((instr >> 16) & 0x3);
		}

		constexpr auto unpack_rgstr_1(const ivm_instr_t instr) -> const std::uint8_t {

			return static_cast<std::uint8_t>((instr >> 12) & 0xF);
		}

		constexpr auto unpack_rgstr_2(const ivm_instr_t instr) -> const std::uint8_t {

			return static_cast<std::uint8_t>((instr >> 8) & 0xF);
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

	#define DEFINE_INSTR(name, opcode, unary)							\
	template <typename T = internal::ivm_default_t, typename TT = void>				\
	constexpr auto name(internal::ivm_rgstr_t rgstr_1 = 0, internal::ivm_rgstr_t rgstr_2 = 0)	\
		-> internal::ivm_instr_t {								\
													\
		return internal::build_instr<T, TT>(rgstr_1, unary ? rgstr_1 : rgstr_2, opcode);	\
	}

	DEFINE_INSTR(MOV, internal::IVM_MOV, 0)
	DEFINE_INSTR(ADD, internal::IVM_ADD, 0)
	DEFINE_INSTR(SUB, internal::IVM_SUB, 0)
	DEFINE_INSTR(XOR, internal::IVM_XOR, 0)
	DEFINE_INSTR(LEA, internal::IVM_LEA, 0)
	DEFINE_INSTR(CMP, internal::IVM_CMP, 0)
	DEFINE_INSTR(JMP, internal::IVM_JMP, 1)
	DEFINE_INSTR(JNE, internal::IVM_JNE, 1)
	DEFINE_INSTR(CALL, internal::IVM_CALL, 1)
	DEFINE_INSTR(PUSH, internal::IVM_PUSH, 1)
	DEFINE_INSTR(POP, internal::IVM_POP, 1)
	DEFINE_INSTR(RET, internal::IVM_RET, 0)

	#undef DEFINE_INSTR

	//

	auto ivm(const std::vector<internal::ivm_instr_t>& prgm, bool dbg = true) -> void {

		auto prgm_counter = 0, zf = 0;

		internal::ivm_rgstr_t rgstr[8] = { 0 };

		internal::ivm_stack_t stack[IVM_STACK_SIZE] = { 0 };

		rgstr[7] = (reinterpret_cast<std::uintptr_t>(&stack[0])) + IVM_STACK_SIZE;

		while (prgm_counter < prgm.size()) {

			const auto instr = prgm[prgm_counter] ^ IVM_SEED;

			const auto opcode = internal::unpack_opcode(instr);

			const auto dref = internal::unpack_dref(instr);
			const auto immv = internal::unpack_immv(instr);
			const auto size = internal::unpack_size(instr);

			const auto rgstr_1 = internal::unpack_rgstr_1(instr);
			const auto rgstr_2 = internal::unpack_rgstr_2(instr);

			auto src = (opcode == internal::IVM_POP)

				? reinterpret_cast<internal::ivm_value_t*>(rgstr[7])

				: (immv ? const_cast<internal::ivm_value_t*>(&prgm[prgm_counter + 1])

					: reinterpret_cast<internal::ivm_value_t*>(&rgstr[rgstr_2]));

			auto dst = (opcode == internal::IVM_PUSH)

				? reinterpret_cast<internal::ivm_value_t*>(rgstr[7] -= size)

				: reinterpret_cast<internal::ivm_value_t*>(&rgstr[rgstr_1]);

			if (dref & internal::IVM_DREF_SRC) {

				src = *reinterpret_cast<internal::ivm_value_t**>(src);
			}

			if (dref & internal::IVM_DREF_DST) {

				dst = *reinterpret_cast<internal::ivm_value_t**>(dst);
			}
		
			switch (opcode) {

			case internal::IVM_PUSH:
			case internal::IVM_POP:
			case internal::IVM_MOV: {

				memcpy(dst, src, size);

				break;
			}
			case internal::IVM_ADD:
			case internal::IVM_SUB:
			case internal::IVM_AND:
			case internal::IVM_XOR: {

				auto bitwise_op = [opcode](auto& a, const auto& b) -> void {

					switch (opcode) {

					case internal::IVM_ADD: a += b; break;
					case internal::IVM_SUB: a -= b; break;
					case internal::IVM_AND: a &= b; break;
					case internal::IVM_XOR: a ^= b; break;

					}
				};

				bitwise_op(*dst, *src);

				break;
			}
			case internal::IVM_LEA: {

				*dst = reinterpret_cast<internal::ivm_value_t>(src);

				break;
			}
			case internal::IVM_CMP: {

				zf = ((*dst - *src) == 0);

				break;
			}
			case internal::IVM_JMP:
			case internal::IVM_JNE: {

				if (opcode == internal::IVM_JMP || zf == 0) {

					prgm_counter = *reinterpret_cast<int*>(src);

					continue;
				}

				break;
			}
			case internal::IVM_CALL: {

				rgstr[0] = ((internal::ivm_rgstr_t(*)(...))*src)(

					rgstr[1],
					rgstr[2],
					rgstr[3],
					rgstr[4],
					rgstr[5],
					rgstr[6]
				);

				break;
			}
			case internal::IVM_RET: {

				prgm_counter = static_cast<int>(prgm.size());

				break;
			}
			}

			if (opcode == internal::IVM_POP) {

				rgstr[7] += size;
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
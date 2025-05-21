/*

	Copyright 2025 Ben Urquhart

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at:

		http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.

*/

#pragma once

#include <iostream>
#include <vector>
#include <cstring>
#include <cstdint>

namespace ivm {

	constexpr std::uint64_t IVM_SEED = 5432167890123456789;

	constexpr std::size_t IVM_STACK_SIZE = 1024;

	constexpr std::uint8_t IVM_DEBUG = 1;

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

			if (std::is_pointer<T>::value && std::is_pointer<TT>::value) {

				return IVM_DREF_CPY;
			}
			else if (std::is_pointer<T>::value && std::is_same<TT, void>::value) {

				return IVM_DREF_SRC;
			}
			else if (std::is_pointer<T>::value) {

				return IVM_DREF_DST;
			}
			else if (std::is_pointer<TT>::value) {

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

		template <typename T>
		auto cast_impl(std::true_type, T v) -> ivm_instr_t {

			return reinterpret_cast<ivm_instr_t>(v);
		}

		template <typename T>
		auto cast_impl(std::false_type, T v) -> ivm_instr_t {

			return static_cast<ivm_instr_t>(v);
		}

		auto interpreter(const std::vector<ivm_instr_t>& prgm) -> void {

			auto prgm_counter = 0, zf = 0;

			ivm_rgstr_t rgstr[8] = { 0 };

			ivm_stack_t stack[IVM_STACK_SIZE] = { 0 };

			rgstr[7] = (reinterpret_cast<std::uintptr_t>(&stack[0])) + IVM_STACK_SIZE;

			while (prgm_counter < prgm.size()) {

				const auto instr = prgm[prgm_counter] ^ IVM_SEED;

				const auto opcode = unpack_opcode(instr);

				if(opcode == IVM_RET) {

					break;
				}

				const auto dref = unpack_dref(instr);
				const auto immv = unpack_immv(instr);
				const auto size = unpack_size(instr);

				const auto rgstr_1 = unpack_rgstr_1(instr);
				const auto rgstr_2 = unpack_rgstr_2(instr);

				auto src = (opcode == IVM_POP)

					? reinterpret_cast<ivm_value_t*>(rgstr[7])

					: (immv ? const_cast<ivm_value_t*>(&prgm[prgm_counter + 1])

						: reinterpret_cast<ivm_value_t*>(&rgstr[rgstr_2]));

				auto dst = (opcode == IVM_PUSH)

					? reinterpret_cast<ivm_value_t*>(rgstr[7] -= size)

					: reinterpret_cast<ivm_value_t*>(&rgstr[rgstr_1]);

				if (dref & IVM_DREF_SRC) {

					src = *reinterpret_cast<ivm_value_t**>(src);
				}

				if (dref & IVM_DREF_DST) {

					dst = *reinterpret_cast<ivm_value_t**>(dst);
				}

				switch (opcode) {

				case IVM_PUSH:
				case IVM_POP:
				case IVM_MOV: {

					memcpy(dst, src, size);

					break;
				}
				case IVM_ADD:
				case IVM_SUB:
				case IVM_AND:
				case IVM_XOR: {

					auto bitwise_op = [opcode](auto& a, const auto& b) -> void {

						switch (opcode) {

						case IVM_ADD: a += b; break;
						case IVM_SUB: a -= b; break;
						case IVM_AND: a &= b; break;
						case IVM_XOR: a ^= b; break;

						}
					};

					bitwise_op(*dst, *src);

					break;
				}
				case IVM_LEA: {

					*dst = reinterpret_cast<ivm_value_t>(src);

					break;
				}
				case IVM_CMP: {

					zf = ((*dst - *src) == 0);

					break;
				}
				case IVM_JMP:
				case IVM_JNE: {

					if (opcode == IVM_JMP || zf == 0) {

						prgm_counter = *reinterpret_cast<int*>(src);

						continue;
					}

					break;
				}
				case IVM_CALL: {

					const auto ptr = (void**)rgstr[7];

					rgstr[0] = ((ivm_rgstr_t(*)(...))*src)(ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5], ptr[6], ptr[7], ptr[8], ptr[10]);

					break;
				}
				}

				if (opcode == IVM_POP) {

					rgstr[7] += size;
				}

				prgm_counter += immv ? 2 : 1;
			};

			if (IVM_DEBUG) {

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

	template <typename... Args>
	void ivm(Args... args) {

		auto universal_cast = [](auto val) -> internal::ivm_instr_t {

			using T = decltype(val);

			return internal::cast_impl(std::is_pointer<T>{}, val);
		};

		internal::interpreter({ universal_cast(args)... });
	}
}
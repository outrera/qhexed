/*
 * This is a syntax test file
 */

struct TestStruct {
	u8 testField
}

struct TestStruct2 {
	s2 someSize
	struct InnerStruct {
		s8 testField2
	}
	struct InnerStruct2 {
		s4 testField
		u4 someOtherField[21+testField+someSize]
	}
}


struct Elf32_Ehdr {
	struct ident {
		char magic[4]
		u1 class
		u1 data
		u1 version
		u1 os_abi
		u1 abi_version
		u1 pad[7]
	}
	u2 e_type
	u2 e_machine
	u2 e_version
	u4 e_entry
	u4 e_phoff
	u4 e_shoff
	u4 e_flags
	u2 e_ehsize
	u2 e_phentsize
	u2 e_phnum
	u2 e_shentsize
	u2 e_shnum
	u2 e_shtrndx
}

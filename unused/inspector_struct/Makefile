sp :=
sp +=
_walk = $(if $1,$(wildcard /$(subst $(sp),/,$1)/$2) $(call _walk,$(wordlist 2,$(words $1),x $1),$2))
_find = $(firstword $(call _walk,$(strip $(subst /, ,$1)),$2))
_ROOT := $(patsubst %/root.mak,%,$(call _find,$(CURDIR),root.mak))

include $(_ROOT)/root.mak
$(call DEPENDS_ON,lib/hex)
include $(_ROOT)/top.mak

SRCS				:= HexGraph.cc main.cc
BINARY				:= StructureInspector
#BINARY_EXT			:= $(_SOEXT)
BINARY_EXT			:= $(_EXEEXT)
#SHARED				:= true
SHARED				:= false
DEPS				:= $(lib_hex_BINARY)

# -DNAMESPACE=StructParserNS renames all 'NAMESPACE' ids to 'StructParserNS'
ADD_CPPFLAGS := -I$($(_MODULE_NAME)_OUTPUT) -DNAMESPACE=StructParserNS

include $(_ROOT)/bottom.mak


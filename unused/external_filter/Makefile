sp :=
sp +=
_walk = $(if $1,$(wildcard /$(subst $(sp),/,$1)/$2) $(call _walk,$(wordlist 2,$(words $1),x $1),$2))
_find = $(firstword $(call _walk,$(strip $(subst /, ,$1)),$2))
_ROOT := $(patsubst %/root.mak,%,$(call _find,$(CURDIR),root.mak))

include $(_ROOT)/root.mak
$(call DEPENDS_ON,lib/hex)
include $(_ROOT)/top.mak

SRCS				:=	Plugin.cc \
						FilterDialog.cc \
						FilterEditDialog.cc \
						Filter.cc \


BINARY				:= ExternalFilter
BINARY_EXT			:= $(_SOEXT)
SHARED				:= true
DEPS				:= $(lib_hex_BINARY)

include $(_ROOT)/bottom.mak


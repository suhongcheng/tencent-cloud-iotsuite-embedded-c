#############################################################
# Required variables for each makefile
# Discard this section from all parent makefiles
# Expected variables (with automatic defaults):
#   CSRCS (all "C" files in the dir)
#   SUBDIRS (all subdirs with a Makefile)
#   GEN_LIBS - list of libs to be generated ()
#   GEN_IMAGES - list of object file images to be generated ()
#   GEN_BINS - list of binaries to be generated ()
#   COMPONENTS_xxx - a list of libs/objs in the form
#     subdir/lib to be extracted and rolled up into
#     a generated lib/image xxx.a ()
#
TARGET = eagle
#FLAVOR = release
FLAVOR = debug

#EXTRA_CCFLAGS += -u

ifndef PDIR # {
GEN_IMAGES= eagle.app.v6.out
GEN_BINS= eagle.app.v6.bin
SPECIAL_MKTARGETS=$(APP_MKTARGETS)
SUBDIRS=    src \
	    user \
            external

endif # } PDIR

LDDIR = $(SDK_PATH)/ld

CCFLAGS += -Os \
	   -DJSMN_PARENT_LINKS \
	   -DENABLE_LOG_TRACE \
	   -DENABLE_LOG_DEBUG \
	   -DENABLE_LOG_INFO \
	   -DENABLE_LOG_WARN \
	   -DENABLE_LOG_ERROR \
	   -DENABLE_LOG_FATAL \

# enable tls
# CCFLAGS += -DENABLE_TLS


TARGET_LDFLAGS =		\
	-nostdlib		\
	-Wl,-EL \
	--longcalls \
	--text-section-literals

ifeq ($(FLAVOR),debug)
    TARGET_LDFLAGS += -g -O2
endif

ifeq ($(FLAVOR),release)
    TARGET_LDFLAGS += -g -O0
endif

COMPONENTS_eagle.app.v6 = \
    user/libtc_iot_user.a  \
    src/libtc_iot_suite.a \
    external/libjsmn.a  \
    external/libmqtt_pack.a \


LINKFLAGS_eagle.app.v6 = \
	-L$(SDK_PATH)/lib        \
	-Wl,--gc-sections   \
	-nostdlib	\
    -T$(LD_FILE)   \
	-Wl,--no-check-sections	\
    -u call_user_start	\
	-Wl,-static						\
	-Wl,--start-group					\
	-lcirom \
	-lgcc					\
	-lhal					\
	-lphy	\
	-lpp	\
	-lnet80211	\
	-lwpa	\
	-lmain	\
	-lfreertos	\
	-llwip	\
	-lssl	\
	-ldriver \
	-lmirom\
	-lmbedtls               \
    -lopenssl               \
	-ljson  \
	-lsmartconfig \
	-lspiffs	\
	$(DEP_LIBS_eagle.app.v6)					\
	-lcrypto \
	-Wl,--end-group
	
DEPENDS_eagle.app.v6 = \
                $(LD_FILE) \
                $(LDDIR)/eagle.rom.addr.v6.ld

#############################################################
# Configuration i.e. compile options etc.
# Target specific stuff (defines etc.) goes in here!
# Generally values applying to a tree are captured in the
#   makefile at its root level - these are then overridden
#   for a subtree within the makefile rooted therein
#

#UNIVERSAL_TARGET_DEFINES =		\

# Other potential configuration flags include:
#	-DTXRX_TXBUF_DEBUG
#	-DTXRX_RXBUF_DEBUG
#	-DWLAN_CONFIG_CCX
CONFIGURATION_DEFINES =	-DICACHE_FLASH -DMQTT_DIRECT

DEFINES +=				\
	$(UNIVERSAL_TARGET_DEFINES)	\
	$(CONFIGURATION_DEFINES)

DDEFINES +=				\
	$(UNIVERSAL_TARGET_DEFINES)	\
	$(CONFIGURATION_DEFINES)


#############################################################
# Recursion Magic - Don't touch this!!
#
# Each subtree potentially has an include directory
#   corresponding to the common APIs applicable to modules
#   rooted at that subtree. Accordingly, the INCLUDE PATH
#   of a module can only contain the include directories up
#   its parent path, and not its siblings
#
# Required for each makefile to inherit from the parent
#

#PDIR := ../$(PDIR)

INCLUDES := $(INCLUDES) -I $(PDIR)include -I include -I $(SDK_PATH)/include
INCLUDES += -I $(PDIR)include/platform/freertos

sinclude $(SDK_PATH)/Makefile

.PHONY: FORCE
FORCE:

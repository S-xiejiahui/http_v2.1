# 编译的文件
FILES += c-web-server
FILES += video-server
OTHER_FILES := html

FIND_BIN  := $(shell find . -name bin)
SUBDIRS   := $(shell dirname `find . -name "makefile" -o -name "Makefile"`)
DIR_INDEX  := $(patsubst %, %/, $(SUBDIRS))

# get local ip addr
LOACL_IP_ADDR = $(shell ifconfig | grep inet | grep -v inet6 | grep -v 127 | sed 's/^[ \t]*//g' | cut -d ' ' -f2)
# Use regular expressions to match files's ip addr
JS_FILE_IP_ADDR = $(shell grep -E -o '(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)' js/app.js)
OLD_IP = $(shell echo $(JS_FILE_IP_ADDR)| cut -d ' ' -f 1)

# 编译整个工程
all:
ifeq ($(FIND_BIN), )
	mkdir bin
	@for i in $(FILES);do \
		make -C $$i $@ || exit $?; \
	done
else
	@for i in $(FILES);do \
		make -C $$i $@ || exit $?; \
	done
endif

# 工程初始化
init:
	make -C $(OTHER_FILES)

info:
#	@sed -i 's/$(OLD_IP)/$(LOACL_IP_ADDR)/' $(files)
	@echo "-----------------local_IP-------------------\n"
	@echo "\t      \033[31m$(LOACL_IP_ADDR)\033[0m\n"
	@echo "--------------Url Download Path-------------\n"
	@echo "  \033[36m$(shell git remote -v | grep -v fetch | awk '{print $$2}')\033[0m"
	@echo "\n--------------------------------------------"

# 将编译好的文件进行打包
image:
ifeq ($(FIND_BIN), )
	@echo "\033[33m--The bin file could not be found---\033[0m"
	@echo "Please perform \033[31m'make' or 'make all'\033[0m"
else
	@tar -cvf image.tar bin html > /dev/null
	@cp image.tar /tftpboot
	@echo "\033[33m---The compressed file is successfully created---\033[0m"	
endif

# 编译单个目录
$(DIR_INDEX):
%_only:
	@case "$(@)" in \
	*/*) d=`expr $(@) : '\(.*\)_only'`; \
	     make -C $$d; \
	esac

# 清理单个目录
%_clean:
	@case "$(@)" in \
	*/*) d=`expr $(@) : '\(.*\)_clean'`; \
	     make -C $$d clean; \
	esac

# 清理所有目录
clean:
	@for i in $(FILES);do \
		make clean -C $$i; \
	done
	-rm -rf bin/*

# 将工程复原成编译前的状态
recovery: 
	-rm -rf bin
	@for i in $(FILES);do \
		make clean -C $$i; \
		rm -rf $$i/obj; \
	done



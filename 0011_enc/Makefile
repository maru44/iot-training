# ESP_COM = /dev/tty.usbserial-1130

.PHONY: summary
summary:
	espefuse.py -p ${ESP_COM} summary

ENCRYPT_FLASH_KEY=./__keys__/flash/key.bin

.PHONY: gen_flash_key
gen_flash_key:
	espsecure.py generate_flash_encryption_key ${ENCRYPT_FLASH_KEY}

# build & upload後に行う

# デバイスのflash_encryptionの書き換え
.PHONY: burn_flash_key
burn_flash_key:
	espefuse.py -p ${ESP_COM} burn_key flash_encryption ${ENCRYPT_FLASH_KEY}

.PHONY: dump
dump:
	espefuse.py --port ${ESP_COM} dump

# デバイスのflash_encryptionの有効化
# 初回のactivateが 0x01
.PHONY: activate_key
activate_key:
	espefuse.py -p ${ESP_COM} burn_efuse FLASH_CRYPT_CNT 0x01 && \
		espefuse.py -p ${ESP_COM} burn_efuse FLASH_CRYPT_CONFIG 15

.PHONY: encrypt_base
encrypt_base:
	espsecure.py encrypt_flash_data --keyfile ${ENCRYPT_FLASH_KEY} \
	  --address ${MEM_OFFSET} --output ./.pio/build/encrypted/${TARGET} ./.pio/build/upesy_wroom/${TARGET}

# ビルドファイルの暗号化
.PHONY: encrypt
encrypt:
	@$(MAKE) encrypt_base TARGET=bootloader.bin MEM_OFFSET=0x1000 && \
	$(MAKE) encrypt_base TARGET=firmware.bin MEM_OFFSET=0x10000 && \
	$(MAKE) encrypt_base TARGET=partitions.bin MEM_OFFSET=0x8000

.PHONY: encrypt_signed_base
encrypt_signed_base:
	espsecure.py encrypt_flash_data --keyfile ${ENCRYPT_FLASH_KEY} \
	  --address ${MEM_OFFSET} --output ./.pio/build/signed_encrypted/${TARGET} ./.pio/build/signed/${TARGET}

.PHONY: encrypt_signed
encrypt_signed:
	@$(MAKE) encrypt_signed_base TARGET=bootloader.bin MEM_OFFSET=0x1000 && \
	$(MAKE) encrypt_signed_base TARGET=firmware.bin MEM_OFFSET=0x10000 && \
	$(MAKE) encrypt_signed_base TARGET=partitions.bin MEM_OFFSET=0x8000

# デバイスに暗号化したものをアップロード
.PHONY: upload
upload:
	esptool.py --chip esp32 --port ${ESP_COM} --baud 115200 \
		--before default_reset --after hard_reset write_flash -z \
		--flash_mode dio --flash_freq 80m --flash_size detect \
		0x1000 ./.pio/build/encrypted/bootloader.bin \
		0x10000 ./.pio/build/encrypted/firmware.bin \
		0x8000 ./.pio/build/encrypted/partitions.bin

# デバイスに暗号化したものをアップロード
.PHONY: upload_without_boot
upload_without_boot:
	esptool.py --chip esp32 --port ${ESP_COM} --baud 115200 \
		--before default_reset --after hard_reset write_flash -z \
		--flash_mode dio --flash_freq 80m --flash_size detect \
		0x10000 ./.pio/build/encrypted/firmware.bin \
		0x8000 ./.pio/build/encrypted/partitions.bin

# デバイスに署名&暗号化したものをアップロード
.PHONY: upload_signed_first
upload_signed_first:
	esptool.py --chip esp32 --port ${ESP_COM} --baud 115200 \
		--before default_reset --after hard_reset write_flash -z \
		--flash_mode dio --flash_freq 80m --flash_size detect \
		0x1000 ./.pio/build/signed_encrypted/bootloader.bin \
		0x10000 ./.pio/build/signed_encrypted/firmware.bin \
		0x8000 ./.pio/build/signed_encrypted/partitions.bin

# デバイスに署名&暗号化したものをアップロード
.PHONY: upload_signed
upload_signed:
	esptool.py --chip esp32 --port ${ESP_COM} --baud 115200 \
		--before default_reset --after hard_reset write_flash -z \
		--flash_mode dio --flash_freq 80m --flash_size detect \
		0x10000 ./.pio/build/signed_encrypted/firmware.bin \
		0x8000 ./.pio/build/signed_encrypted/partitions.bin

####################
# 
# boot
# 
####################

BOOT_PRIVATE=./__keys__/boot/secure_boot.pem
BOOT_PUBLIC=./__keys__/boot/secure_boot_public.pem
BOOT_DIGEST=./__keys__/boot/digested_secure_boot.bin

# bootloader用のkey生成
.PHONY: gen_boot_key
gen_boot_key:
	espsecure.py generate_signing_key --version 2 ${BOOT_PRIVATE}

.PHONY: gen_boot_pub_key
gen_pub_key:
	espsecure.py extract_public_key --version 2 --keyfile ${BOOT_PRIVATE} ${BOOT_PUBLIC}

.PHONY:	sign_base
sign_base:
	espsecure.py sign_data --version 2 --keyfile ${BOOT_PRIVATE} --output ./.pio/build/signed/${TARGET} ./.pio/build/upesy_wroom/${TARGET}

.PHONY: sign
sign:
	@$(MAKE) sign_base TARGET=bootloader.bin && \
	$(MAKE) sign_base TARGET=firmware.bin && \
	$(MAKE) sign_base TARGET=partitions.bin

# ref: https://github.com/espressif/esp-idf/issues/6916#issuecomment-825599437

.PHONY: burn_boot_key
burn_boot_key:
	espefuse.py -p ${ESP_COM} burn_key_digest ${BOOT_PRIVATE}

.PHONY: activate_boot
activate_boot:
	espefuse.py -p ${ESP_COM} --chip esp32 burn_efuse ABS_DONE_1

# .PHONY: burn_digest
# burn_digest:
# 	espefuse.py burn_key_digest --show-sensitive-info BLOCK_KEY0 ${BOOT_PRIVATE} SECURE_BOOT_DIGEST0

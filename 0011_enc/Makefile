# ESP_COM = /dev/tty.usbserial-1130

.PHONY: summary
summary:
	espefuse.py -p ${ESP_COM} summary

# build & upload後に行う

# デバイスのflash_encryptionの書き換え
.PHONY: write_key
write_key:
	espefuse.py -p ${ESP_COM} burn_key flash_encryption ./__encrypt_key__/key.bin

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
	espsecure.py encrypt_flash_data --keyfile ./__encrypt_key__/key.bin \
	  --address ${MEM_OFFSET} --output ./.pio/build/encrypted/${TARGET} ./.pio/build/upesy_wroom/${TARGET}

# ビルドファイルの暗号化
.PHONY: encrypt
encrypt:
	@$(MAKE) encrypt_base TARGET=bootloader.bin MEM_OFFSET=0x1000 && \
	$(MAKE) encrypt_base TARGET=firmware.bin MEM_OFFSET=0x10000 && \
	$(MAKE) encrypt_base TARGET=partitions.bin MEM_OFFSET=0x8000

# デバイスに暗号化したものをアップロード
.PHONY: upload
upload:
	esptool.py --chip esp32 --port ${ESP_COM} --baud 115200 \
		--before default_reset --after hard_reset write_flash -z \
		--flash_mode dio --flash_freq 80m --flash_size detect \
		0x1000 ./.pio/build/encrypted/bootloader.bin \
		0x10000 ./.pio/build/encrypted/firmware.bin \
		0x8000 ./.pio/build/encrypted/partitions.bin

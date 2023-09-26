# How to Secure Boot and Encryption

## Preparation

Prepare for secure boot and flash encryption

### Generate Flash Encryption Key

flash encryption 用の鍵を生成

```shell
make gen_flash_key
```

### Generate Secure Boot Key

secure boot 用の鍵を生成

```shell
make gen_boot_key
```

<!-- secure boot用の共有鍵を生成
```shell
make gen_boot_pub_key
``` -->

## Sign and Encrypt Build Files

```shell
make sign
```

```shell
make encrypt_signed
```

## Burn Keys for ESP32

```shell
make burn_flash_key
```

```shell
make burn_boot_key
```

## Upload

```shell
make upload_signed_first
```

## Activate Boot Loader

```shell
make activate_boot
```

## Upload

```shell
make upload_signed
```

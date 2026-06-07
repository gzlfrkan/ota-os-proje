# OTA Toolchain Firmware Analizi

Bu çalışma, Contiki-NG ortamında üretilmiş farklı platformlara ait firmware dosyalarının toolchain araçlarıyla analiz edilmesini amaçlamaktadır. Analizlerde MSP430 tabanlı `.z1` / `.sky` firmware dosyaları, ARM Cortex-M4F / CC1352R tabanlı `.simplelink` firmware dosyası ve Cooja/native ortamına ait `.cooja` binary dosyası incelenmiştir.

Çalışma kapsamında firmware dosyaları; binary kimliği, bellek kullanımı, sembol/fonksiyon yapısı, string/metadata bilgileri, assembly çıktıları, ELF yapısı, interrupt/donanım bağlantıları, ağ katmanı, TSCH/MAC yapısı, sensör/peripheral kullanımı, matematiksel işlem izleri, güç/performans davranışı, coverage/profiling durumu, reverse engineering çıkarımları, compiler/linker etkileri, binary dönüşümleri, library/archive içerikleri, Contiki-NG özel yapıları, güvenlik/robustness durumu ve karşılaştırmalı firmware özellikleri açısından değerlendirilmiştir.

---

## İçindekiler

1. [Analiz Edilen Firmware Dosyaları](#analiz-edilen-firmware-dosyaları)
2. [Kullanılan Araçlar](#kullanılan-araçlar)
3. [Özet Karşılaştırma](#özet-karşılaştırma)
4. [1. Binary Kimlik Analizi](#1-binary-kimlik-analizi)
5. [2. Bellek Kullanım Analizi](#2-bellek-kullanım-analizi)
6. [3. Symbol / Function Analizi](#3-symbol--function-analizi)
7. [4. String ve Metadata Analizi](#4-string-ve-metadata-analizi)
8. [5. Assembly / Instruction Analizi](#5-assembly--instruction-analizi)
9. [6. Source-Level Mapping Analizi](#6-source-level-mapping-analizi)
10. [7. ELF Yapısı Analizi](#7-elf-yapısı-analizi)
11. [8. Interrupt ve Donanım Analizi](#8-interrupt-ve-donanım-analizi)
12. [9. Networking Analizi](#9-networking-analizi)
13. [10. Wireless / TSCH Analizi](#10-wireless--tsch-analizi)
14. [11. Sensor ve Peripheral Analizi](#11-sensor-ve-peripheral-analizi)
15. [12. Algoritma / DSP / Matematiksel Analiz](#12-algoritma--dsp--matematiksel-analiz)
16. [13. Güç ve Performans Analizi](#13-güç-ve-performans-analizi)
17. [14. Coverage ve Profiling Analizi](#14-coverage-ve-profiling-analizi)
18. [15. Reverse Engineering Analizi](#15-reverse-engineering-analizi)
19. [16. Compiler ve Optimization Analizi](#16-compiler-ve-optimization-analizi)
20. [17. Linker ve Build Sistemi Analizi](#17-linker-ve-build-sistemi-analizi)
21. [18. Binary Transformation Analizi](#18-binary-transformation-analizi)
22. [19. Library ve Archive Analizi](#19-library-ve-archive-analizi)
23. [20. Contiki-NG Özel Analizler](#20-contiki-ng-özel-analizler)
24. [21. Güvenlik ve Robustness Analizi](#21-güvenlik-ve-robustness-analizi)
25. [22. Karşılaştırmalı Firmware Analizi](#22-karşılaştırmalı-firmware-analizi)
26. [23. Eğitimsel Reverse Engineering Görevleri](#23-eğitimsel-reverse-engineering-görevleri)
27. [Genel Sonuç](#genel-sonuç)

---

## Analiz Edilen Firmware Dosyaları

Çalışmada ana `rpl-udp` dizinindeki firmware dosyaları ve `extra-firmwares` dizinindeki ek örnek firmware dosyaları incelenmiştir.

### Ana dizindeki dosyalar

| Dosya | Platform / Mimari | Genel rol |
|---|---|---|
| `new-firmware.z1` | Z1 / MSP430 | OTA kapsamında incelenen yeni firmware imajı |
| `udp-client.z1` | Z1 / MSP430 | UDP istemci uygulaması |
| `udp-server.z1` | Z1 / MSP430 | UDP sunucu uygulaması |

### Ek firmware dosyaları

| Dosya | Platform / Mimari | Genel rol |
|---|---|---|
| `base-demo.simplelink` | ARM / CC13xx-CC26xx / SimpleLink | ARM tabanlı base demo firmware |
| `hardworker.z1` | Z1 / MSP430 | Sensör, LED, UDP/RPL davranışı içeren yoğun test firmware’i |
| `hello-world.sky` | Sky / MSP430 | Hello world örneği |
| `hello-world.z1` | Z1 / MSP430 | Hello world örneği |
| `mtype5756516.cooja` | x86-64 / Cooja native | Cooja/native simülasyon binary dosyası |
| `nullnet-broadcast.z1` | Z1 / MSP430 | NullNet broadcast örneği |
| `nullnet-unicast-u50.sky` | Sky / MSP430 | NullNet örneği |
| `nullnet-unicast.sky` | Sky / MSP430 | NullNet unicast örneği |
| `nullnet-unicast.z1` | Z1 / MSP430 | NullNet unicast örneği |
| `nullnet-unicast_eleco.sky` | Sky / MSP430 | NullNet unicast varyantı |
| `udp-client.sky` | Sky / MSP430 | UDP istemci örneği |
| `udp-client.z1` | Z1 / MSP430 | Ek UDP istemci örneği |
| `udp-server.sky` | Sky / MSP430 | UDP sunucu örneği |

---

## Kullanılan Araçlar

Analizlerde MSP430, ARM ve native ELF dosyaları için uygun toolchain araçları kullanılmıştır.

| Araç | Kullanım amacı |
|---|---|
| `file` | Dosya tipi, mimari ve ELF kimliği belirleme |
| `sha256sum` | Dosya bütünlük/hash değeri üretme |
| `stat`, `ls -lh` | Dosya boyutu inceleme |
| `msp430-size`, `arm-none-eabi-size`, `size` | `text`, `data`, `bss`, toplam bellek kullanımı |
| `msp430-readelf`, `arm-none-eabi-readelf`, `readelf` | ELF header, section header, program header, relocation analizi |
| `msp430-objdump`, `arm-none-eabi-objdump`, `objdump` | Section ve disassembly analizi |
| `msp430-nm`, `arm-none-eabi-nm`, `nm` | Sembol, fonksiyon, ISR ve değişken analizi |
| `msp430-strings`, `arm-none-eabi-strings`, `strings` | Okunabilir string, log, metadata ve davranış ipucu analizi |
| `msp430-objcopy`, `arm-none-eabi-objcopy`, `objcopy` | ELF → binary / HEX dönüşümü, section extraction |
| `msp430-strip`, `arm-none-eabi-strip`, `strip` | Debug ve sembol bilgilerini kaldırma |
| `msp430-cpp` | Contiki macro/preprocessor expansion inceleme |
| `msp430-gcov`, `msp430-gprof` | Coverage/profiling için araç uygunluğu kontrolü |

> Not: `gcov` ve `gprof` araçları sistemde mevcut olsa da hazır firmware dosyaları runtime coverage/profiling çıktısı üretmediği için gerçek çalışma zamanı profil ölçümü yapılamamıştır. Bu nedenle ilgili başlıklarda statik analiz sonuçları yorumlanmıştır.

---

## Özet Karşılaştırma

Aşağıdaki tablo firmware dosyalarının genel bellek kullanımını göstermektedir. `dec` değeri `text + data + bss` toplamıdır.

| Dosya | text | data | bss | dec | Yorum |
|---|---:|---:|---:|---:|---|
| `nullnet-broadcast.z1` | 17866 | 166 | 2240 | 20272 | En küçük MSP430 örneklerden biri |
| `nullnet-unicast-u50.sky` | 17793 | 310 | 2814 | 20917 | Hafif NullNet örneği |
| `nullnet-unicast_eleco.sky` | 17883 | 4626 | 2998 | 25507 | Data alanı görece yüksek |
| `nullnet-unicast.z1` | 28097 | 2488 | 2632 | 33217 | NullNet unicast |
| `nullnet-unicast.sky` | 29171 | 4426 | 3086 | 36683 | Sky NullNet unicast |
| `hello-world.z1` | 41512 | 328 | 5676 | 47516 | Basit Contiki örneği |
| `extra-firmwares/udp-client.z1` | 42542 | 336 | 5888 | 48766 | UDP client |
| `udp-server.z1` | 42585 | 336 | 5866 | 48787 | UDP server |
| `udp-client.z1` | 42871 | 336 | 5922 | 49129 | UDP client |
| `hello-world.sky` | 42237 | 324 | 6714 | 49275 | Sky hello-world |
| `udp-server.sky` | 43000 | 330 | 6968 | 50298 | Sky UDP server |
| `udp-client.sky` | 43386 | 330 | 7024 | 50740 | Sky UDP client |
| `new-firmware.z1` | 71715 | 336 | 5706 | 77757 | OTA kapsamında incelenen büyük MSP430 firmware |
| `hardworker.z1` | 73564 | 374 | 5698 | 79636 | Yoğun işlem/sensör/UDP/RPL içeren MSP430 firmware |
| `base-demo.simplelink` | 71393 | 1408 | 12968 | 85769 | ARM / SimpleLink firmware |
| `mtype5756516.cooja` | 324415 | 9488 | 292376 | 626279 | Cooja/native x86-64 binary, gömülü firmware değil |

Genel olarak NullNet örnekleri UDP/RPL kullanan firmware’lere göre daha küçüktür. `new-firmware.z1` ve `hardworker.z1` daha fazla ağ, sensör ve event-driven yapı içerdiği için daha büyük çıkmıştır. `base-demo.simplelink` ARM/SimpleLink sürücüleri nedeniyle MSP430 örneklerinden farklıdır. `mtype5756516.cooja` ise gömülü firmware değil, Cooja/native simülasyon binary’sidir.

---

# 1. Binary Kimlik Analizi

## Amaç

Firmware dosyalarının hangi mimari ve platform için üretildiğini, ELF format özelliklerini, debug sembol durumunu, strip edilip edilmediğini ve dosya bütünlüğünü belirlemek.

## Kullanılan araçlar

- `file`
- `sha256sum`
- `stat`
- `msp430-readelf`
- `arm-none-eabi-readelf`
- `readelf`

## Bulgular

Analiz edilen dosyalar üç ana sınıfa ayrılmıştır:

1. **MSP430 ELF firmware dosyaları:** `.z1` ve `.sky` uzantılı dosyaların büyük kısmı `ELF 32-bit LSB executable, TI msp430` formatındadır.
2. **ARM/SimpleLink firmware:** `base-demo.simplelink`, `ELF 32-bit LSB executable, ARM, EABI5` formatındadır.
3. **Cooja/native binary:** `mtype5756516.cooja`, `ELF 64-bit LSB shared object, x86-64` formatındadır.

MSP430 dosyalarının çoğu `statically linked`, `with debug_info` ve `not stripped` olarak görünmektedir. Bu durum firmware dosyalarının analiz için sembol ve debug bilgisi içerdiğini göstermektedir.

## Yorum

Binary kimlik analizi sonucunda `.z1` / `.sky` firmware dosyalarının MSP430 tabanlı olduğu, `base-demo.simplelink` dosyasının ARM/SimpleLink platformuna ait olduğu ve `.cooja` dosyasının gerçek gömülü firmware değil, native simülasyon binary’si olduğu anlaşılmıştır.

---

# 2. Bellek Kullanım Analizi

## Amaç

Firmware dosyalarının Flash ve RAM kullanımını, `.text`, `.data`, `.bss` dağılımını ve büyük veri/kod alanlarını incelemek.

## Kullanılan araçlar

- `msp430-size`
- `arm-none-eabi-size`
- `size`
- `msp430-readelf`
- `msp430-nm`

## Bulgular

`msp430-size` çıktılarında:

- `.text`: Kod ve salt-okunur sabit verileri temsil eder.
- `.data`: Başlangıç değeri verilmiş global/static verileri temsil eder.
- `.bss`: Başlangıç değeri verilmemiş global/static veriler için RAM’de ayrılan alanı temsil eder.
- `dec`: Toplam bellek kullanımını gösterir.

`new-firmware.z1` için bellek kullanımı:

```text
text    data    bss    dec
71715   336     5706   77757
```

`udp-client.z1` ve `udp-server.z1` yaklaşık 49 KB toplam kullanımdayken, `new-firmware.z1` yaklaşık 77 KB toplam kullanım göstermiştir. NullNet örnekleri ise daha küçüktür.

## Yorum

`new-firmware.z1` dosyasının büyük olmasının temel nedeni `.text` alanının yüksek olmasıdır. Bu da daha fazla kod, ağ stack bileşeni, RPL/UDP/6LoWPAN yapısı, sensör ve sürücü fonksiyonu içerdiğini gösterir. `bss` alanı RAM kullanımını etkilediği için gömülü sistemlerde önemlidir.

---

# 3. Symbol / Function Analizi

## Amaç

Firmware içindeki fonksiyonları, global/static değişkenleri, ISR fonksiyonlarını, Contiki process entry’lerini, radio driver fonksiyonlarını ve networking callback’lerini tespit etmek.

## Kullanılan araçlar

- `msp430-nm`
- `arm-none-eabi-nm`
- `nm`
- `msp430-objdump`
- `readelf`

## Bulgular

MSP430 firmware dosyalarında aşağıdaki sembol grupları görülmüştür:

```text
main
_reset_vector__
__isr_*
process_thread_*
ctimer_process
etimer_process
tcpip_process
uip_process
rpl_dag_*
rpl_icmp6_*
cc2420_*
uart0_*
i2c_*
watchdog_*
```

`udp-client.z1` içinde `process_thread_udp_client_process`, `udp-server.z1` içinde `process_thread_udp_server_process` bulunmuştur. Bu, firmware rollerinin sembol düzeyinde doğrulanabildiğini göstermektedir.

`base-demo.simplelink` dosyasında ise MSP430 yerine ARM/SimpleLink tarafına ait şu tip semboller görülmüştür:

```text
RF_open
RF_scheduleCmd
RF_runCmd
UARTCC26XX_*
SPICC26X2DMA_*
I2CCC26XX_*
WatchdogCC26XX_*
PINCC26XX_*
PowerCC26X2_*
```

## Yorum

Sembol tablosu sayesinde firmware’in çalışma mantığı hakkında önemli çıkarımlar yapılabilmiştir. Dosyalar strip edilmediği için fonksiyon isimleri korunmuştur. Bu durum reverse engineering ve eğitimsel analiz açısından büyük avantaj sağlamaktadır.

---

# 4. String ve Metadata Analizi

## Amaç

Firmware içindeki okunabilir stringleri inceleyerek debug mesajları, process isimleri, network protokolleri, sensör isimleri ve davranış ipuçlarını çıkarmak.

## Kullanılan araçlar

- `msp430-strings`
- `arm-none-eabi-strings`
- `strings`

## Bulgular

`new-firmware.z1` içinde şu tür stringler görülmüştür:

```text
Starting Contiki-NG-release/v4.8-625-g8518cbaff-dirty
Hello, EK-D103
Hello world process
created a new RPL DAG
RPL Lite
Receiving UDP packet
udp: bad checksum
ADXL345 sensor
TMP102 sensor
Button
```

`udp-client.z1` için:

```text
Simple UDP process
UDP client
Sending request
Client received response
```

`udp-server.z1` için:

```text
Simple UDP process
UDP server
Received request
Sending response
```

NullNet firmware’lerde:

```text
NullNet broadcast example
NullNet unicast example
nullnet
Sending
Received
```

`base-demo.simplelink` içinde:

```text
CC13xx/CC26xx base demo
Button HAL process
RF Scheduler Process
PowerCC26X2
TimerPCC26XX_nortos.c
```

## Yorum

String analizi, firmware davranışının hızlı sınıflandırılması için en kullanışlı yöntemlerden biridir. UDP client/server, NullNet broadcast/unicast, RPL kullanımı, sensör varlığı ve platform türü stringlerden büyük ölçüde anlaşılmıştır.

---

# 5. Assembly / Instruction Analizi

## Amaç

Firmware içindeki instruction sequence, function prologue/epilogue, register kullanımı, loop, branch, ISR akışı, busy-wait ve compiler davranışlarını incelemek.

## Kullanılan araçlar

- `msp430-objdump`
- `arm-none-eabi-objdump`
- `objdump`

## Bulgular

MSP430 dosyalarında aşağıdaki instruction örnekleri görülmüştür:

```text
pushm.a
calla
mov
cmp
tst
jnz
jz
jmp
reta
reti
```

`reti` interrupt dönüşü için önemli bir instruction’dır. `calla` komutu fonksiyon çağrılarını, `cmp`, `tst`, `jnz`, `jz`, `jmp` ise branch/loop davranışlarını göstermektedir.

`new-firmware.z1` içinde `input`, `output`, `uip_process`, `tcpip_ipv6_output`, `rpl_process_dio`, `csma_output_packet` gibi büyük ağ fonksiyonlarının assembly düzeyinde karmaşık branch yapıları içerdiği görülmüştür.

## Yorum

Assembly analizi, özellikle ağ paketi işleme, RPL, 6LoWPAN/IPHC ve radio driver fonksiyonlarının karmaşıklığını göstermektedir. Gömülü firmware’de yüksek branch sayısı genellikle state machine, protokol işleme ve hata kontrol akışlarıyla ilişkilidir.

---

# 6. Source-Level Mapping Analizi

## Amaç

Debug bilgisi varsa adreslerin kaynak dosya ve satır bilgileriyle ilişkilendirilebilir olup olmadığını incelemek.

## Kullanılan araçlar

- `msp430-addr2line`
- `msp430-objdump -S`
- `msp430-readelf`
- `strings`

## Bulgular

Firmware dosyalarında debug sectionları görülmüştür:

```text
.debug_aranges
.debug_info
.debug_abbrev
.debug_line
.debug_frame
.debug_str
.debug_loc
.debug_ranges
.symtab
.strtab
```

Bu bölümler firmware dosyalarının debug bilgisi taşıdığını göstermektedir.

MSP430 tarafında `crt0ivtbl.S`, `libgcc.S`, `printf.c`, `sprintf.c`, `vuprintf.c` gibi kaynak dosya izleri görülmüştür. ARM/SimpleLink tarafında ise `NoRTOS.c`, `TimerPCC26XX_nortos.c`, `HwiPCC26XX_nortos.c`, `PowerCC26X2_nortos.c`, `RFCC26X2_multiMode.c` gibi kaynak izleri bulunmuştur.

## Yorum

Debug sectionlarının varlığı, adres → kaynak eşlemesi için uygun ortam sağlar. Ancak doğru ve tam source-level mapping için build sırasında kullanılan kaynak dosyaların aynı path yapısıyla erişilebilir olması gerekir.

---

# 7. ELF Yapısı Analizi

## Amaç

ELF header, section header, program header, symbol table, debug sections, startup section, vector table ve initialization routine yapılarını incelemek.

## Kullanılan araçlar

- `msp430-readelf`
- `arm-none-eabi-readelf`
- `readelf`
- `msp430-objdump`

## `new-firmware.z1` temel ELF bilgisi

| Özellik | Değer |
|---|---|
| Class | ELF32 |
| Endian | Little endian |
| Type | EXEC |
| Machine | Texas Instruments MSP430 |
| Entry point | `0x3100` |
| Program header sayısı | 6 |
| Section header sayısı | 21 |
| Debug bilgisi | Var |
| Strip durumu | Not stripped |

Önemli sectionlar:

```text
.far.text
.text
.rodata
.data
.bss
.noinit
.vectors
.debug_*
.symtab
.strtab
```

`.text` bölümü `0x3100` adresinden başlamaktadır. Entry point değerinin de `0x3100` olması, yürütmenin ana kod bölümüyle uyumlu olduğunu göstermektedir.

`.far.text` bölümü `0x10000` adresindedir. Bu bölüm, firmware’in ek/uzak çalıştırılabilir kod alanı kullandığını gösterir.

`.vectors` bölümü `0xffc0` adresinde yer almaktadır ve MSP430 interrupt/reset vector yapısı açısından kritiktir.

## ARM / SimpleLink farkı

`base-demo.simplelink` dosyasında MSP430’daki `.vectors` yerine ARM tarafında `.resetVecs` section’ı görülmüştür. Ayrıca `.ARM.exidx`, `.ARM.attributes`, `.heap`, `.data`, `.bss` gibi ARM’a özgü bölümler vardır.

## Yorum

ELF yapısı, firmware’in belleğe nasıl yerleştiğini ve hangi sectionların runtime için kritik olduğunu göstermektedir. MSP430 firmware’lerde `.vectors`, ARM/SimpleLink firmware’de `.resetVecs` özellikle boot/reset akışı için önemlidir.

---

# 8. Interrupt ve Donanım Analizi

## Amaç

Interrupt vector table, GPIO erişimleri, timer interruptları, UART/I2C interruptları, radio interrupt handlerları, ADC/sensor erişimleri ve register düzeyi donanım izlerini incelemek.

## Kullanılan araçlar

- `msp430-objdump`
- `msp430-readelf`
- `msp430-nm`
- ARM karşılıkları

## Bulgular

MSP430 firmware’lerde şu semboller görülmüştür:

```text
_reset_vector__
__isr_*
cc2420_interrupt
cc2420_timerb1_interrupt
i2c_rx_interrupt
i2c_tx_interrupt
port1_isr
timera0
timera1
uart0_rx_interrupt
watchdog_interrupt
```

Donanım/peripheral tarafında:

```text
ADC12
DMA
I2C
UART
SPI
LED
Button sensor
ADXL345 sensor
TMP102 sensor
CC2420 radio driver
```

ARM/SimpleLink tarafında:

```text
HwiP_*
PIN_hwi
RF_hwi*
SPICC26X2DMA_hwiFxn
UARTCC26XX_hwiIntFxn
WatchdogCC26XX_*
resetISR
resetVectors
```

## Yorum

Interrupt/donanım analizi sayesinde firmware’in hangi çevre birimleriyle etkileştiği ve hangi ISR fonksiyonlarını içerdiği çıkarılmıştır. MSP430 tarafında CC2420 radio, I2C, UART, timer ve watchdog interruptları öne çıkarken, ARM tarafında HwiP/RF/PIN/UART/SPI/I2C/Watchdog driver katmanları görülmüştür.

---

# 9. Networking Analizi

## Amaç

Firmware’in ağ protokollerini, unicast/broadcast kullanımını, IPv6 stack, RPL routing, MAC interaction, packet buffer, neighbor table ve radio transmission akışını incelemek.

## Kullanılan araçlar

- `msp430-nm`
- `msp430-objdump`
- `msp430-strings`
- ARM karşılıkları

## Bulgular

`new-firmware.z1` içinde şu ağ sembolleri görülmüştür:

```text
tcpip_process
uip_process
uip_ds6_*
rpl_dag_*
rpl_icmp6_*
dio_input
dao_input
dis_input
sicslowpan_driver
packetbuf_*
queuebuf_*
link_stats_*
netstack_*
csma_driver
cc2420_driver
```

String tarafında:

```text
RPL Lite
created a new RPL DAG
DIO
DAO
DIS
IPv6 addresses
IPHC
compression
uncompression
```

`udp-client.z1` içinde `process_thread_udp_client_process`, `simple_udp_register`, `simple_udp_sendto` sembolleri görülmüştür. `udp-server.z1` içinde `process_thread_udp_server_process` sembolü vardır.

NullNet örneklerinde ise `nullnet_driver`, `nullnet_buf`, `nullnet_len`, `nullnet_set_input_callback`, `process_thread_nullnet_example_process` sembolleri bulunmuştur.

## Yorum

UDP/RPL/IPv6 kullanan firmware dosyaları NullNet örneklerine göre daha karmaşıktır. `new-firmware.z1`, `udp-client.z1`, `udp-server.z1` ve `hardworker.z1` ağ stack’i açısından daha kapsamlıdır. NullNet örnekleri daha hafif bir ağ soyutlaması kullanmaktadır.

---

# 10. Wireless / TSCH Analizi

## Amaç

TSCH slot operation, channel hopping, MAC timing, schedule management, radio timing loops, CSMA/TSCH farkları ve radio driver kullanımını incelemek.

## Kullanılan araçlar

- `msp430-objdump`
- `msp430-nm`
- `msp430-strings`
- ARM karşılıkları

## Bulgular

MSP430 `.z1` ve `.sky` firmware dosyalarında ağırlıklı olarak şu yapılar görülmüştür:

```text
cc2420_*
csma_driver
frame802154_*
framer_802154
packetbuf_*
queuebuf_*
linkaddr_*
link_stats_*
mac_sequence_*
sicslowpan_driver
```

Bu durum, bu firmware’lerde baskın yapının **CSMA + IEEE 802.15.4 + CC2420 radio driver** olduğunu göstermektedir.

`mtype5756516.cooja` içinde ise TSCH ile ilgili semboller görülmüştür:

```text
process_thread_tsch_process
process_thread_tsch_send_eb_process
tsch_queue_*
tsch_schedule
tsch_broadcast_address
```

`base-demo.simplelink` tarafında CC2420 yerine `RF_*` sembolleri öne çıkmıştır:

```text
RF_open
RF_scheduleCmd
RF_runCmd
RF_getRssi
RF_setTxPower
RF_yield
```

## Yorum

MSP430 firmware’lerde aktif yapı daha çok CSMA/802.15.4 iken, TSCH izleri özellikle Cooja/native binary tarafında belirginleşmektedir. ARM/SimpleLink firmware ise CC2420 yerine SimpleLink RF sürücülerini kullanmaktadır.

---

# 11. Sensor ve Peripheral Analizi

## Amaç

Button, LED, UART, SPI, I2C, ADC, sensor polling, interrupt-driven sensor logic ve peripheral initialization sequence yapılarını incelemek.

## Kullanılan araçlar

- `msp430-objdump`
- `msp430-nm`
- `msp430-strings`
- ARM karşılıkları

## Bulgular

MSP430/Z1 firmware’lerde şu semboller görülmüştür:

```text
adxl345
adxl345_default_settings
button_sensor
leds_arch_get
leds_arch_init
leds_arch_set
leds_on
leds_off
sensors_process
sensors_changed
tmp102
tmp102_init
tmp102_read_temp_x100
i2c_*
spi_*
uart0_*
ADC12
DMA
```

`hardworker.z1` içinde ek olarak:

```text
led_process
sensor_process
process_thread_led_process
process_thread_sensor_process
leds_toggle
```

String tarafında:

```text
[SENSOR] Button pressed
[SENSOR] Received UDP-triggered poll
[LED] Toggled
Sensor Event P.ID:06
LED Toggle P.ID:03
```

ARM/SimpleLink tarafında:

```text
PINCC26XX_*
PIN_setInterrupt
PIN_setOutputValue
button_hal_process
button_hal_press_event
batmon_sensor
```

## Yorum

MSP430 firmware’lerde ADXL345 ivmeölçer, TMP102 sıcaklık sensörü, button, LED, I2C/SPI/UART gibi çevre birimleri izlenmiştir. `hardworker.z1`, sensör ve LED davranışı açısından en belirgin örnektir. ARM/SimpleLink tarafında peripheral erişimi daha çok TI driver katmanı üzerinden yapılmaktadır.

---

# 12. Algoritma / DSP / Matematiksel Analiz

## Amaç

Floating-point, fixed-point, trigonometric computation, multiply/divide routines, software floating-point emulation, DSP benzeri loop’lar, matrix/signal processing izleri ve computational hotspot bölgelerini incelemek.

## Kullanılan araçlar

- `msp430-objdump`
- `msp430-nm`
- `msp430-gprof`
- ARM karşılıkları

## Bulgular

MSP430 firmware’lerde aşağıdaki integer arithmetic helper routine sembolleri görülmüştür:

```text
__mulsi3
__divhi3
__divsi3
__udivhi3
__udivsi3
__udivdi3
__xabi_udivmod64
```

Bazı NullNet unicast varyantlarında `__floatundisf` gibi floating-point dönüşüm helper izleri görülmüştür. Ancak genel firmware setinde belirgin `sin`, `cos`, `sqrt`, matrix, FFT, FIR/IIR gibi ağır DSP/trigonometri sembolleri baskın değildir.

Statik olarak büyük ve maliyetli olabilecek fonksiyonlar:

```text
uip_process
input
output
tcpip_ipv6_output
rpl_process_dio
rpl_icmp6_dio_output
csma_output_packet
vuprintf
```

## Yorum

Firmware’lerde ağır DSP veya matrix işleme davranışı bulunmamaktadır. Matematiksel açıdan en belirgin yapı integer multiply/divide helper routine’leridir. Computational hotspot adayları daha çok ağ paketi işleme, RPL, uIP ve debug/printf fonksiyonlarıdır.

---

# 13. Güç ve Performans Analizi

## Amaç

Low-power mode, CPU-intensive function’lar, busy-wait, sleep/wakeup flow, timer usage, radio duty cycle, ISR yoğunluğu, Flash/RAM efficiency ve energy-heavy bölgeleri incelemek.

## Kullanılan araçlar

- `msp430-gprof`
- `msp430-objdump`
- `msp430-size`
- ARM karşılıkları

## Bulgular

MSP430 firmware’lerde şu semboller görülmüştür:

```text
energest_init
energest_flush
platform_idle
clock_*
ctimer_*
etimer_*
rtimer_*
stimer_*
watchdog_*
cc2420_rssi
cc2420_get_txpower
cc2420_set_txpower
queuebuf_*
packetbuf_*
memb_*
```

Busy-wait/delay açısından:

```text
clock_delay
wait_ready
wait_for_transmission
wait_for_status
```

Stringlerde performans/robustness açısından önemli olabilecek ifadeler:

```text
could not allocate queuebuf
could not allocate packet
not enough packetbuf space
dropping packet
failed to allocate
```

ARM/SimpleLink tarafında:

```text
PowerCC26X2_*
Power_sleep
Power_idleFunc
Power_setConstraint
Power_releaseConstraint
RF_getRssi
RF_getTxPower
RF_setTxPower
RF_yield
```

## Yorum

MSP430 firmware’lerde enerji ve performans davranışı timer, watchdog, radio, queuebuf/packetbuf ve Energest sembolleriyle ilişkilidir. ARM/SimpleLink firmware’de ise PowerCC26X2 ve RF sürücüleri güç yönetimi açısından daha belirgin yapı sunmaktadır.

---

# 14. Coverage ve Profiling Analizi

## Amaç

Function call frequency, execution hotspot, unused branch, rarely executed path, test coverage, critical execution path ve runtime bottleneck tespiti yapmak.

## Kullanılan araçlar

- `msp430-gcov`
- `msp430-gprof`
- ARM karşılıkları

## Bulgular

Proje dizininde `.gcno`, `.gcda`, `gmon.out`, `.prof` gibi runtime coverage/profiling çıktıları bulunmamıştır. Araçlar sistemde mevcut olsa da firmware dosyaları profiling/coverage desteğiyle derlenmediği için gerçek çalışma zamanı profil analizi yapılamamıştır.

Statik analizde büyük fonksiyonlar hotspot adayı olarak değerlendirilmiştir:

```text
input
output
uip_process
tcpip_ipv6_output
rpl_process_dio
rpl_icmp6_dio_output
csma_output_packet
vuprintf
```

`mtype5756516.cooja` içinde `__gmon_start__` sembolü görülmüştür; ancak tek başına bu sembol gerçek profiling çıktısı olduğu anlamına gelmez.

## Yorum

Gerçek coverage/profiling için firmware’in `-pg` veya gcov flagleriyle yeniden derlenmesi ve çalışma sonrası `gmon.out`, `.gcda`, `.gcno` gibi dosyaların üretilmesi gerekir. Bu çalışmada runtime profiling yerine statik sembol büyüklükleri yorumlanmıştır.

---

# 15. Reverse Engineering Analizi

## Amaç

Firmware behavior recovery, unknown firmware classification, feature inference, protocol inference, ISR purpose discovery, hardware interaction recovery, state machine/scheduler/event-flow reconstruction ve network role inference yapmak.

## Kullanılan araçlar

- `msp430-objdump`
- `msp430-nm`
- `msp430-readelf`
- `msp430-strings`
- ARM karşılıkları

## Bulgular

String ve sembol analizleriyle şu sınıflandırma yapılmıştır:

| Dosya | Reverse engineering sonucu |
|---|---|
| `new-firmware.z1` | RPL/ağ, sensör, button ve hello-world process izleri içeren geniş firmware |
| `udp-client.z1` | UDP client |
| `udp-server.z1` | UDP server |
| `hardworker.z1` | Sensör, LED, UDP-triggered poll, simulated UDP packet içeren yoğun test firmware |
| `nullnet-broadcast.z1` | NullNet broadcast |
| `nullnet-unicast*.z1/.sky` | NullNet unicast/broadcast varyantları |
| `base-demo.simplelink` | CC13xx/CC26xx SimpleLink ARM firmware |
| `mtype5756516.cooja` | Cooja/native simülasyon binary |

ISR/hardware inference tarafında MSP430 firmware’lerde:

```text
__isr_*
_reset_vector__
cc2420_interrupt
cc2420_timerb1_interrupt
i2c_rx_interrupt
i2c_tx_interrupt
uart0_rx_interrupt
watchdog_interrupt
```

ARM/SimpleLink tarafında:

```text
resetISR
resetVectors
HwiP_*
RF_hwi*
PIN_hwi
UARTCC26XX_hwiIntFxn
SPICC26X2DMA_hwiFxn
```

## Yorum

Firmware davranışı yalnızca binary üzerinden büyük ölçüde geri kazanılabilmiştir. Stringler genel rolü hızlıca verirken, sembol tablosu process, protocol, ISR ve hardware etkileşimlerini doğrulamıştır.

---

# 16. Compiler ve Optimization Analizi

## Amaç

Compiler/toolchain izlerini, optimization davranışını, inlining, dead code elimination, constant folding, loop optimization, register allocation, branch optimization, macro expansion ve preprocessor etkilerini incelemek.

## Kullanılan araçlar

- `msp430-gcc`
- `msp430-cpp`
- `msp430-objdump`
- `msp430-readelf`
- ARM karşılıkları

## Bulgular

MSP430 firmware’lerde `.comment` section içinde şu compiler izi görülmüştür:

```text
GCC: (GNU) 4.7.2 20120920 (mspgcc dev 20120911)
```

ARM/SimpleLink firmware’de:

```text
GNU Arm Embedded Toolchain
arm-none-eabi
GCC 9.3.1 / 7.2.1 / 4.9.3 izleri
```

Optimization izi olarak şu sembol kalıpları görülmüştür:

```text
.part
.isra
.constprop
```

Örnekler:

```text
accm_read_axis.part.0
entry_is_allowed.isra.2
nbr_get_bit.constprop.3
packet_input.part.2
```

Fonksiyon sayıları:

| Dosya | Fonksiyon sayısı |
|---|---:|
| `new-firmware.z1` | 530 |
| `udp-client.z1` | 525 |
| `udp-server.z1` | 525 |
| `base-demo.simplelink` | 761 |
| `mtype5756516.cooja` | 1109 |

## Yorum

`.part`, `.isra`, `.constprop` gibi semboller compiler optimization etkisini göstermektedir. Hazır ELF üzerinden kesin `-O0`, `-O2`, `-Os` bilgisi çıkarılamasa da optimizasyon izleri ve fonksiyon isimleri üzerinden derleyici davranışı hakkında yorum yapılabilmiştir.

---

# 17. Linker ve Build Sistemi Analizi

## Amaç

Section placement, link order, static library linkage, startup code, linker script behavior, vector placement, symbol resolution ve relocation behavior incelemek.

## Kullanılan araçlar

- `msp430-ld`
- `msp430-ar`
- `msp430-ranlib`
- `msp430-readelf`
- ARM karşılıkları

## Bulgular

Proje Makefile’ında:

```text
CONTIKI_PROJECT = udp-client udp-server
```

Ayrıca dizinde `slot-a.ld` ve `slot-b.ld` linker script dosyaları görülmüştür. Bu durum OTA/slot mantığı için özel bellek yerleşimi kullanıldığını düşündürmektedir.

`new-firmware.z1` section placement:

```text
.text      0x3100
.rodata    0xc870
.data      0x1100
.bss       0x1250
.noinit    0x2898
.vectors   0xffc0
.far.text  0x10000
```

Startup sembolleri:

```text
__do_copy_data
__do_clear_bss
__init_stack
__vectors_start
_reset_vector__
__data_start
__bss_start
__stack
```

Relocation analizi sonucunda final firmware dosyalarında dinamik relocation bulunmamıştır:

```text
There are no relocations in this file.
```

## Yorum

Firmware statik olarak linklenmiştir. Linker, kod, sabit veri, RAM verisi, vektör alanı ve uzak kod alanını ayrı section/segmentler halinde yerleştirmiştir. `.data` başlangıçta Flash’tan RAM’e kopyalanır, `.bss` ise başlangıçta sıfırlanır.

---

# 18. Binary Transformation Analizi

## Amaç

ELF → HEX conversion, ELF → binary conversion, section extraction, symbol stripping, debug removal, firmware minimization ve binary patch preparation işlemlerini test etmek.

## Kullanılan araçlar

- `msp430-objcopy`
- `msp430-strip`
- `arm-none-eabi-objcopy`
- `arm-none-eabi-strip`

## Bulgular

`new-firmware.z1` için:

| İşlem | Sonuç |
|---|---:|
| Orijinal ELF | 127K |
| Raw binary | 71K |
| Intel HEX | 198K |
| Strip debug | 103K |
| Strip all | 72K |
| `.vectors` extraction | 64 byte |
| `.resetVecs` extraction | 0 byte |

MSP430 firmware’lerde boot/interrupt vector bölümü `.vectors` olarak çıkarılmıştır. ARM/SimpleLink firmware’de ise `.resetVecs` 64 byte olarak çıkarılmıştır.

`base-demo.simplelink` için:

| İşlem | Sonuç |
|---|---:|
| Orijinal ELF | 649K |
| Raw binary | 352K |
| Strip all | 226K |
| `.resetVecs` extraction | 64 byte |

## Yorum

ELF dosyaları debug/sembol/metadata içerdiği için raw binary’den daha büyüktür. `strip` işlemleri dosya boyutunu azaltır ancak `text/data/bss` çalışma zamanı bellek değerlerini değiştirmez. MSP430 ve ARM tarafında reset/vector section isimleri farklıdır.

---

# 19. Library ve Archive Analizi

## Amaç

Static library içeriği, object file extraction, archive symbol table ve linked module analizi yapmak.

## Kullanılan araçlar

- `msp430-ar`
- `msp430-gcc-ar`
- `msp430-ranlib`
- `msp430-nm`
- ARM karşılıkları

## Bulgular

Proje içinde ayrı `.a` archive dosyası tespit edilmemiştir. Ancak build dizinlerinde çok sayıda `.o` object dosyası bulunmuştur:

```text
cc2420.o
csma.o
rpl-*.o
tcpip.o
simple-udp.o
udp-client.o
udp-server.o
watchdog.o
xmem.o
adxl345.o
tmp102.o
sha-256.o
sicslowpan.o
ota-metadata.o
```

MSP430 final firmware içinde runtime/helper sembolleri görülmüştür:

```text
__divhi3
__divsi3
__modhi3
__mulsi3
__udivdi3
memcpy
memmove
memset
printf
snprintf
vuprintf
```

ARM/SimpleLink tarafında:

```text
__aeabi_ldivmod
__aeabi_uldivmod
__udivmoddi4
memcpy
memmove
memset
printf
snprintf
vsnprintf
```

## Yorum

Firmware tek bir kaynak dosyadan oluşmamaktadır; Contiki-NG, platform sürücüleri, ağ stack’i, runtime helper fonksiyonları ve uygulama object dosyalarının linklenmesiyle oluşmuştur. Ayrı `.a` archive dosyası bulunmasa da final ELF içinde library/runtime fonksiyonları mevcuttur.

---

# 20. Contiki-NG Özel Analizler

## Amaç

`PROCESS_THREAD` recovery, protothread expansion, event-driven scheduler, `etimer/ctimer` kullanımı, `PROCESS_BEGIN/END`, `PROCESS_YIELD`, `NETSTACK`, `packetbuf` lifecycle, `uIP` callback chain ve Rime/NullNet kullanımını incelemek.

## Kullanılan araçlar

- `msp430-cpp`
- `msp430-objdump`
- `msp430-nm`
- `grep`
- ARM karşılıkları

## Bulgular

Kaynak kod seviyesinde `udp-client.c` içinde:

```c
PROCESS(...)
AUTOSTART_PROCESSES(...)
PROCESS_THREAD(...)
PROCESS_BEGIN()
PROCESS_WAIT_EVENT_UNTIL(...)
simple_udp_register(...)
simple_udp_sendto(...)
etimer_set(...)
NETSTACK_ROUTING.node_is_reachable()
```

`udp-server.c` içinde:

```c
PROCESS(...)
AUTOSTART_PROCESSES(...)
PROCESS_THREAD(...)
PROCESS_BEGIN()
NETSTACK_ROUTING.root_start()
simple_udp_register(...)
```

Binary sembol tarafında:

```text
autostart_processes
process_init
process_run
process_start
process_post
process_poll
process_thread_*
ctimer_process
etimer_process
tcpip_process
uip_process
```

Timer yapıları:

```text
clock_time
ctimer_*
etimer_*
rtimer_*
stimer_*
timer_*
```

NETSTACK/uIP/packetbuf yapıları:

```text
packetbuf_*
queuebuf_*
netstack_*
tcpip_*
uip_*
sicslowpan_driver
rpl_*
```

## Yorum

Contiki-NG uygulamaları klasik işletim sistemi thread’leri gibi değil, event-driven protothread mantığıyla çalışmaktadır. Bu yapı kaynak kodda `PROCESS_THREAD`, binary’de ise `process_thread_*`, `process_run`, `process_post`, `ctimer`, `etimer`, `tcpip_process` sembolleriyle izlenebilmektedir.

---

# 21. Güvenlik ve Robustness Analizi

## Amaç

Hardcoded credential, debug backdoor, buffer handling, unsafe memory access, stack-heavy routines, potential overflow bölgeleri, assert/debug remnants ve information leakage stringlerini aramak.

## Kullanılan araçlar

- `msp430-strings`
- `msp430-objdump`
- `msp430-readelf`
- `msp430-nm`
- ARM karşılıkları

## Bulgular

Credential/backdoor aramasında gerçek anlamda şu tür açık gizli bilgi bulunmamıştır:

```text
password
token
apikey
login
admin
backdoor
private key
```

`root` kelimesi bazı yerlerde görülse de bu RPL/DODAG root bağlamındadır, kullanıcı hesabı veya şifre anlamında değildir.

Robustness stringleri:

```text
could not allocate queuebuf
could not allocate packet
could not allocate neighbor
neighbor table full
malformed packet
invalid fragment offset
bad checksum
not enough packetbuf space
dropping packet
failed to allocate
```

Buffer/memory ile ilişkili semboller:

```text
memcpy
memmove
memset
printf
snprintf
vuprintf
packetbuf_*
queuebuf_*
uipbuf_*
stack_check_*
```

Büyük/kritik fonksiyon adayları:

```text
input
output
uip_process
vuprintf
frag_buf
rpl_process_dio
tcpip_ipv6_output
dio_input
```

Disassembly’de bazı buffer fonksiyonlarında sınır kontrolleri görülmüştür:

```text
packetbuf_copyto
packetbuf_hdralloc
uipbuf_set_len
uipbuf_add_ext_hdr
```

## Yorum

Açık hardcoded credential/backdoor bulgusu yoktur. Firmware’de ağ/buffer hatalarına karşı çok sayıda hata kontrol string’i ve boundary check izi vardır. Bununla birlikte `memcpy`, `packetbuf`, `uipbuf`, `queuebuf`, `input/output` gibi fonksiyonlar güvenlik açısından kritik inceleme bölgeleridir.

---

# 22. Karşılaştırmalı Firmware Analizi

## Amaç

Firmware dosyalarını code size, RAM farkı, function count, ISR yoğunluğu, networking complexity, radio stack, symbol farkı, optimization izi ve assembly complexity açısından karşılaştırmak.

## Kullanılan araçlar

- `msp430-size`
- `msp430-nm`
- `msp430-objdump`
- `msp430-strings`
- ARM karşılıkları

## Karşılaştırma bulguları

| Dosya | Function count | ISR count | Networking symbol count | Branch sample count | Rol |
|---|---:|---:|---:|---:|---|
| `new-firmware.z1` | 530 | 40 | 271 | 4293 | RPL/ağ + sensör + hello-world izleri |
| `udp-client.z1` | 525 | 40 | 269 | 3648 | UDP client |
| `udp-server.z1` | 525 | 40 | 269 | 3625 | UDP server |
| `base-demo.simplelink` | 761 | 40 | 347 | 3576 | ARM/SimpleLink base demo |
| `hardworker.z1` | 519 | 40 | 272 | 4373 | Sensör/LED/UDP/RPL test firmware |
| `hello-world.sky` | 474 | 23 | 260 | 3550 | Hello world / Sky |
| `hello-world.z1` | 502 | 40 | 255 | 3586 | Hello world / Z1 |
| `mtype5756516.cooja` | 1109 | 2 | 508 | 4852 | Cooja/native simülasyon |
| `nullnet-broadcast.z1` | 291 | 40 | 67 | 1378 | NullNet broadcast |
| `nullnet-unicast-u50.sky` | 262 | 23 | 71 | 1374 | NullNet |
| `nullnet-unicast.sky` | 300 | 23 | 74 | 2005 | NullNet unicast |
| `nullnet-unicast.z1` | 317 | 40 | 68 | — | NullNet unicast |

## Yorum

UDP/RPL/IPv6 kullanan firmware’lerde networking sembol sayısı NullNet örneklerine göre çok daha yüksektir. NullNet örnekleri daha küçük ve daha sade yapıdadır. `base-demo.simplelink`, ARM/SimpleLink RF sürücüleri nedeniyle MSP430 dosyalarından farklıdır. `mtype5756516.cooja`, en büyük ve sembol açısından en yoğun dosya olsa da gömülü firmware değil, native simülasyon binary’sidir.

---

# 23. Eğitimsel Reverse Engineering Görevleri

## Amaç

Bir firmware’in ne yaptığını bulma, kullandığı protokolü çıkarma, button/LED mapping bulma, ISR’leri tanıma, network role çıkarımı, algoritmik blok tespiti, energy-heavy bölgeleri bulma ve stripped firmware çözümleme gibi eğitimsel görevleri uygulamak.

## Kullanılan araçlar

- `msp430-strings`
- `msp430-nm`
- `msp430-readelf`
- `msp430-strip`
- `msp430-size`
- ARM karşılıkları

## Görev 1: Firmware davranışını stringlerden çıkarma

Örnek çıkarımlar:

| Dosya | Stringlerden çıkarılan rol |
|---|---|
| `udp-client.z1` | `Simple UDP process`, `UDP client` |
| `udp-server.z1` | `Simple UDP process`, `UDP server` |
| `nullnet-broadcast.z1` | `NullNet broadcast example` |
| `nullnet-unicast.z1` | `NullNet unicast example` |
| `hardworker.z1` | `[SENSOR] Button pressed`, `[LED] Toggled`, UDP-triggered poll |
| `base-demo.simplelink` | `CC13xx/CC26xx base demo`, `Button HAL process` |

## Görev 2: Protokol ve network role çıkarımı

UDP dosyalarında:

```text
process_thread_udp_client_process
process_thread_udp_server_process
simple_udp_register
simple_udp_sendto
```

NullNet dosyalarında:

```text
nullnet_driver
nullnet_buf
nullnet_len
nullnet_set_input_callback
process_thread_nullnet_example_process
```

RPL/uIP/6LoWPAN kullanan dosyalarda:

```text
rpl_dag_*
dio_input
dao_input
dis_input
uip_*
tcpip_*
sicslowpan_*
```

## Görev 3: Button / LED / Sensor mapping

MSP430/Z1 dosyalarında:

```text
adxl345
button_sensor
leds_on
leds_off
leds_toggle
tmp102
sensors_process
```

`hardworker.z1` özelinde:

```text
process_thread_led_process
process_thread_sensor_process
[SENSOR] Button pressed
[LED] Toggled
```

## Görev 4: ISR tanıma

MSP430:

```text
.vectors
__isr_*
_reset_vector__
cc2420_timerb1_interrupt
uart0_rx_interrupt
i2c_rx_interrupt
watchdog_interrupt
```

ARM/SimpleLink:

```text
.resetVecs
resetISR
HwiP_*
RF_hwi*
PIN_hwi
UARTCC26XX_hwiIntFxn
```

## Görev 5: Algoritmik / energy-heavy adayları bulma

Aritmetik helperlar:

```text
__mul*
__div*
__udiv*
__mod*
__aeabi*
```

Büyük fonksiyon adayları:

```text
input
output
uip_process
tcpip_ipv6_output
rpl_process_dio
vuprintf
```

## Görev 6: Stripped firmware çözümleme

Bazı firmware dosyalarının strip edilmiş kopyaları oluşturulmuştur. Strip sonrası sembol tablosu kaldırıldığı için `nm` ile fonksiyon isimleri büyük ölçüde kaybolur. Ancak `strings` ile bazı davranış ipuçları hâlâ elde edilebilir:

```text
UDP client
UDP server
RPL Lite
NullNet broadcast example
CC13xx/CC26xx base demo
Button
Sensor
```

## Yorum

Bu görevler, firmware analizinde önce stringlerle hızlı sınıflandırma, ardından sembollerle doğrulama, sonra ELF/section/ISR/hardware detaylarına inme şeklinde bir reverse engineering akışı kurulabileceğini göstermektedir.

---

# Genel Sonuç

Bu çalışmada Contiki-NG tabanlı farklı firmware dosyaları 23 farklı analiz başlığı altında incelenmiştir. Analiz sonucunda:

- `.z1` ve `.sky` dosyalarının MSP430 mimarisi için üretilmiş ELF firmware dosyaları olduğu görülmüştür.
- `base-demo.simplelink` dosyasının ARM / SimpleLink / CC13xx-CC26xx tabanlı firmware olduğu anlaşılmıştır.
- `mtype5756516.cooja` dosyasının gömülü firmware değil, Cooja/native x86-64 simülasyon binary’si olduğu belirlenmiştir.
- UDP/RPL/IPv6 kullanan firmware dosyaları NullNet örneklerine göre daha büyük ve sembol açısından daha karmaşık çıkmıştır.
- `new-firmware.z1`, `.far.text`, `.vectors`, RPL/uIP/6LoWPAN, CC2420 radio, ADXL345, TMP102, button ve sensör altyapısı içermektedir.
- `udp-client.z1` ve `udp-server.z1` rolleri hem string hem sembol düzeyinde doğrulanmıştır.
- `hardworker.z1`, sensör ve LED davranışı açısından en belirgin test firmware’i olarak öne çıkmıştır.
- Debug ve sembol bilgileri firmware dosyalarında mevcut olduğu için statik analiz oldukça verimli yapılabilmiştir.
- Strip işlemi reverse engineering’i zorlaştırsa da stringler üzerinden temel davranış çıkarımı hâlâ yapılabilmektedir.
- Coverage/profiling için gerekli runtime çıktıları bulunmadığından gerçek çalışma zamanı profil analizi yapılamamış; bunun yerine statik sembol ve fonksiyon büyüklüğü analizi yapılmıştır.

Genel olarak firmware dosyalarının yalnızca çalıştırılabilir binary dosyalar olmadığı; mimari, bellek yerleşimi, protokol kullanımı, donanım etkileşimi, compiler/linker etkileri, güvenlik/robustness ipuçları ve reverse engineering açısından çok sayıda bilgi taşıdığı görülmüştür.

---

## Analiz Çıktıları

Çalışma sırasında elde edilen ham analiz çıktıları `readme-analysis/` dizini altında saklanmıştır. Örnek dosyalar:

```text
readme-analysis/01-binary-identity.txt
readme-analysis/02-memory-size.txt
readme-analysis/09-networking-stack.txt
readme-analysis/13-power-performance.txt
readme-analysis/18-binary-transformation.txt
readme-analysis/21-security-robustness.txt
readme-analysis/22-comparative-firmware.txt
readme-analysis/23-educational-re.txt
```

Bu README dosyası, ham terminal çıktılarının tamamını tekrar etmek yerine analizlerden elde edilen önemli bulguları ve yorumları özetlemektedir.

# Tunelování datových přenosů přes DNS dotazy
Autor: Hung Do (xdohun00@fit.vutbr.cz)  
Datum: 23.10.2022

## Popis
Projekt se skládá ze 2 částí: `dns_sender` a `dns_receiver`.  
První program (`dns_sender`) načítá obsah zadaného souboru a jeho obsah poté kódování base64 zabalí 
do DNS dotazu a posílá na příslušný DNS server.
Druhý program (`dns_receiver`) příjímá DNS dotazy a přesměruje je na veřejný Google DNS server.
Pokud ale server přijme dotaz v předem dohodnutém formátu, získaná data od `dns_sender` překonvertuje zpátky na
originální data a zapíše je do příslušného souboru.

## Rozšíření
- podpora IPv6
- odeslání validní odpovědi při přijetí validního DNS dotazu

## Překlad
```bash
git clone https://github.com/hungdojan/ISA_Project.git
cd ISA_Project
# mozno gmake 
make
```

## Spuštění receiver
Musí být volný port 53 a root oprávnění. Pokud uživatel nemá volný port 53 nebo root oprávnění,
je tu možnost úpravy zdrojového kódu a znovu přeložení pomocí `make`.

```
$ # ./dns_receiver {BASE_HOST} {DST_DIRPATH}
$ sudo ./dns_receiver example.com test_dir/
[INIT] ::1
[RECV] test_dir/ipv6.txt         1 32B from ::1
[PARS] test_dir/ipv6.txt '1.VG90byBqZSBwcm9qZWN0IHogSVNBLgo=.example.com'
[CMPL] test_dir/ipv6.txt of 23B
$ cat test_dir/ipv6.txt
Toto je project z ISA.
```

### Překlad pro non-root oprávnění
Stačí upravit proměnnou `DNS_PORT` v souboru `./include/macros.h` na port v rozmezí 1024-65535 (port musí být volný).
```bash
# uprava portu na port 12345
sed -i -E 's/(#define DNS_PORT) [0-9]+/\1 12345/g' include/macros.h
make clean
make
```

## Spuštění sender
```
$ # ./dns_sender [-u UPSTREAM_IP] {BASE_HOST} {DST_FILEPATH} [SRC_FILEPATH]
$ ./dns_sender -u ::1 example.com ipv6.txt
$ Toto je project z ISA.
$ <C-D>
[INIT] ::1
[ENCD] ipv6.txt        32 'VG90byBqZSBwcm9qZWN0IHogSVNBLgo=.example.com'
[SENT] ipv6.txt         2 32B to ::1
[CMPL] ipv6.txt of 23B
```

## Seznam odevzdaných souboru
```
.
├── include
│   ├── base64.h
│   ├── data_queue.h
│   ├── dns_header.h
│   ├── error.h
│   └── macros.h
├── Makefile
├── manual.pdf
├── README.md
├── receiver
│   ├── arguments.c
│   ├── arguments.h
│   ├── data_queue.c
│   ├── dns_receiver_events.c
│   ├── dns_receiver_events.h
│   ├── dns_response.c
│   ├── dns_response.h
│   ├── main.c
│   ├── Makefile
│   └── recv_queue.h
├── sender
│   ├── arguments.c
│   ├── arguments.h
│   ├── data_queue.c
│   ├── dns_query.c
│   ├── dns_query.h
│   ├── dns_sender_events.c
│   ├── dns_sender_events.h
│   ├── main.c
│   ├── Makefile
│   └── send_queue.h
└── shared
    └── base64.c
```

## Použití veřejně dostupných zdrojových souborů
Projekt používá pro kódování dat base64 kódování. Zdrojové soubory byly
zveřejněni skupinou **The Apache Group**. Zdrojové soubory jsou k dostání zde
[base64.h](https://opensource.apple.com/source/QuickTimeStreamingServer/QuickTimeStreamingServer-452/CommonUtilitiesLib/base64.h) a [base64.c](https://opensource.apple.com/source/QuickTimeStreamingServer/QuickTimeStreamingServer-452/CommonUtilitiesLib/base64.c).

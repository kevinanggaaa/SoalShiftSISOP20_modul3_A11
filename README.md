# SoalShiftSISOP20_modul3_A11
## Kevin Angga Wijaya - 05111840000024
## Julius - 05111840000082
![523985](https://user-images.githubusercontent.com/60419316/78343204-b04b8300-75c4-11ea-9141-c19820f7f6ef.jpg)

## Soal 1
**Poke\*ZONE** adalah sebuah game berbasis text terminal mirip dengan Pokemon GO.
Ketentuan permainan sebagai berikut:
 1. Menggunakan IPC-shared memory, thread, fork-exec.
 2. Kalian bebas berkreasi dengan game ini asal tidak konflik dengan requirements yang ada. (Contoh: memberi nama trainer, memberi notifikasi kalau barang di shop sudah penuh, dan lain-lain).
 3. Terdapat 2 code yaitu soal2_traizone.c dan soal2_pokezone.c.
 4. **soal2_traizone.c** mengandung fitur:
    1. Normal Mode (3 Menu)
       1. Cari Pokemon
          1. Jika diaktifkan maka setiap 10 detik akan memiliki 60% chance untuk menemui pokemon bertipe normal, rare, legendary dengan encounter rate sesuai pada tabel di bawah (Gunakan Thread).
          2. Cari pokemon hanya mengatur dia menemukan pokemon atau tidak. Untuk tipe pokemon dan pokemon tersebut shiny atau tidak diatur oleh soal2_pokezone.c.
          3. Opsi “Cari Pokemon” akan berubah jadi “Berhenti Mencari” ketika state mencari pokemon aktif.
          4. State mencari pokemon hanya bisa dihentikan ketika pokemon sudah ditemukan atau trainer memilih untuk berhenti pada menu.
          5. Jika menemui pokemon maka akan masuk ke Capture Mode.
          6. Untuk mempermudah boleh menambah menu go to capture mode untuk berpindah dari mode normal ke mode capture setelah menemukan pokemon dari thread Cari Pokemon.
       2. Pokedex
          1. Melihat list pokemon beserta Affection Point (AP) yang dimiliki.
          2. Maksimal 7 pokemon yang dimiliki.
          3. Jika menangkap lebih dari 7 maka pokemon yang baru saja ditangkap akan langsung dilepas dan mendapatkan pokedollar sesuai dengan tabel dibawah.
          4. Setiap pokemon yang dimiliki, mempunyai Affection Point (AP) dengan initial value 100 dan akan terus berkurang sebanyak -10 AP/10 detik dimulai dari waktu ditangkap (Gunakan Thread).
          5. Jika AP bernilai 0, maka pokemon tersebut memiliki 90% chance untuk lepas tanpa memberikan pokedollar ke trainer atau 10% chance untuk reset AP menjadi 50 AP.
          6. AP tidak akan berkurang jika dalam Capture Mode.
          7. Di Pokedex trainer juga bisa melepas pokemon yang ditangkap dan mendapat pokedollar sesuai dengan tabel dibawah.
          8. Bisa memberi berry ke semua pokemon yang dimiliki untuk meningkatkan AP sebesar +10 (1 berry untuk semua pokemon yang ada di pokedex).
       3. Shop
          1. Membeli item dari soal2_pokezone.
          2. Maksimal masing-masing item yang dapat dibeli dan dipunya oleh trainer adalah 99.
    2. Capture Mode (3 Menu)
       1. Tangkap → Menangkap menggunakan pokeball. Berhasil ditangkap maupun tidak, pokeball di inventory -1 setiap digunakan.
       2. Item → Menggunakan item sesuai dengan tabel item dibawah (hanya lullaby powder).
       3. Keluar → Keluar dari Capture Mode menuju Normal Mode.  
       - Pokemon tersebut memiliki peluang untuk lari dari trainer sesuai dengan persentase escape rate pada tabel dibawah (gunakan thread).
